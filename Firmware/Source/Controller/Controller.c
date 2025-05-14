// Header
#include "Controller.h"
//
// Includes
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"
#include "DeviceProfile.h"
#include "Interrupts.h"
#include "Global.h"
#include "LowLevel.h"
#include "Measurement.h"
#include "SysConfig.h"
#include "math.h"
#include "BCCIxParams.h"
#include "Delay.h"
#include "InitConfig.h"

// Macro
//
#define ABS(a)	(((a) < 0) ? -(a) : (a))

// Types
//
typedef void (*FUNC_AsyncDelegate)();
//
typedef enum __CapBatteryState
{
	CBS_PassiveDischarge 	= 0,
	CBS_ActiveDischarge 	= 1,
	CBS_Charge 				= 2
} CapBatteryState;

// Variables
//
volatile DeviceState CONTROL_State = DS_None;
volatile DeviceSubState CONTROL_SubState = SS_None;
static Boolean CycleActive = false;
static uint16_t ActualBatteryVoltage = 0, TargetBatteryVoltage = 0;
CapBatteryState CONTROL_CapBatteryState = CBS_PassiveDischarge;

volatile Int64U CONTROL_TimeCounter = 0;
Int64U CONTROL_ChargeTimeout = 0, CONTROL_LEDTimeout = 0, CONTROL_SynchronizationTimeout = 0;
Int64U CONTROL_PsBoardDisableTimeout = 0, CONTROL_AfterPulseTimeout = 0;

Int16U CONTROL_ExtInfoCounter = 0;
Int16U CONTROL_ExtInfoData[VALUES_EXT_INFO_SIZE] = {0};

// Forward functions
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void CONTROL_SwitchToFault(Int16U Reason);
void Delay_mS(uint32_t Delay);
void CONTROL_WatchDogUpdate();
void CONTROL_BatteryChargeLogic();
void CONTROL_ResetToDefaultState();
void CONTROL_ResetHardware(bool Discharge);
bool CONTROL_CheckGateRegisterValue();
void CONTROL_HandleSynchronizationTimeout();

// Functions
//
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[EP_COUNT] = {EP16_ExtInfoData};
	Int16U EPSized[EP_COUNT] = {VALUES_EXT_INFO_SIZE};
	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_ExtInfoCounter};
	pInt16U EPDatas[EP_COUNT] = {(pInt16U)CONTROL_ExtInfoData};
	
	// Конфигурация сервиса работы DataTable и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};
	// Инициализация DataTable
	DT_Init(EPROMService, false);
	DT_SaveFirmwareInfo(CAN_SLAVE_NID, 0);

	// Инициализация функций связанных с CAN NodeID
	Int16U NodeID = DataTable[REG_CFG_NODE_ID] ? DataTable[REG_CFG_NODE_ID] : CAN_SLAVE_NID;
	DT_SaveFirmwareInfo(NodeID, 0);
	INITCFG_ConfigCAN(NodeID);

	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive, NodeID);
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Сброс значений
	DEVPROFILE_ResetControlSection();

	CONTROL_ResetToDefaultState();
}
//------------------------------------------

void CONTROL_ResetToDefaultState()
{
	DataTable[REG_FAULT_REASON] = DF_NONE;
	DataTable[REG_DISABLE_REASON] = DF_NONE;
	DataTable[REG_WARNING] = WARNING_NONE;
	DataTable[REG_PROBLEM] = PROBLEM_NONE;
	DataTable[REG_OP_RESULT] = OPRESULT_NONE;
	DataTable[REG_ACTUAL_BAT_VOLTAGE] = 0;
	
	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();

	CONTROL_ResetHardware(true);
	CONTROL_SetDeviceState(DS_None);
	CONTROL_SetDeviceSubState(SS_None);
}
//------------------------------------------

void CONTROL_ResetHardware(bool Discharge)
{
	LL_ExternalLED(false);
	LL_PSBoardOutput(false);
	LL_ForceSYNC(false);
	LL_WriteToGateRegister(0);

	if(Discharge)
	{
		LL_MeanWellRelay(false);
		LL_BatteryDischarge(true);
	}
}
//------------------------------------------

void CONTROL_Idle()
{
	CONTROL_BatteryChargeLogic();
	CONTROL_HandleLEDLogic(Impulse);
	CONTROL_HandleSynchronizationTimeout();

	DEVPROFILE_ProcessRequests();
	CONTROL_WatchDogUpdate();
}
//------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
	*pUserError = ERR_NONE;
	
	switch (ActionID)
	{
		case ACT_ENABLE_POWER:
			{
				if(CONTROL_State == DS_None)
				{
					LL_MeanWellRelay(true);
					CONTROL_InitBatteryChargeProcess();
				}
				else if(CONTROL_State != DS_Ready && CONTROL_State != DS_BatteryCharge)
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_DISABLE_POWER:
			{
				if(CONTROL_State == DS_Ready || CONTROL_State == DS_BatteryCharge)
				{
					CONTROL_ResetToDefaultState();
				}
				else if(CONTROL_State != DS_None)
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			break;
			
		case ACT_FAULT_CLEAR:
			{
				if(CONTROL_State == DS_Fault)
				{
					CONTROL_ResetToDefaultState();
				}
			}
			break;
			
		case ACT_WARNING_CLEAR:
			DataTable[REG_WARNING] = 0;
			break;
			
		case ACT_VOLTAGE_CONFIG:
			{
				if(CONTROL_State == DS_Ready)
					CONTROL_InitBatteryChargeProcess();
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_PULSE_CONFIG:
			{
				if(CONTROL_State == DS_Ready)
				{
					if(!LL_GetSYNCState())
					{
						if (CONTROL_CheckGateRegisterValue())
						{
							DataTable[REG_OP_RESULT] = OPRESULT_NONE;
							LL_WriteToGateRegister(DataTable[REG_GATE_REGISTER]);
							LL_PSBoardOutput(false);
							CONTROL_SynchronizationTimeout = CONTROL_TimeCounter + DataTable[REG_SYNC_WAIT_TIMEOUT];
							CONTROL_PsBoardDisableTimeout = CONTROL_TimeCounter + DataTable[REG_PS_BOARD_DISABLE_TIMEOUT];
							CONTROL_SetDeviceSubState(SS_WaitingSync);
						}
						else
							CONTROL_SwitchToProblem(PROBLEM_GATE_REGISTER);
					}
					else
						CONTROL_SwitchToProblem(PROBLEM_SYNC_LINE);
				}
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			break;

		case ACT_SW_PULSE:
			{
				if(CONTROL_State == DS_Ready)
					LL_PulseSYNC();
				else
					*pUserError = ERR_DEVICE_NOT_READY;
			}
			break;

			// Блок отладочных функций
			
		case ACT_DBG_FAN:
			{
				LL_Fan(true);
				Delay_mS(1000);
				LL_Fan(false);
			}
			break;
			
		case ACT_DBG_DISCHARGE:
			{
				LL_BatteryDischarge(true);
				Delay_mS(1000);
				LL_BatteryDischarge(false);
			}
			break;
			
		case ACT_DBG_EXT_LED:
			{
				LL_ExternalLED(true);
				Delay_mS(1000);
				LL_ExternalLED(false);
			}
			break;
			
		case ACT_DBG_MW_RELAY:
			{
				LL_MeanWellRelay(true);
				Delay_mS(1000);
				LL_MeanWellRelay(false);
			}
			break;
			
		case ACT_DBG_PSBOARD_OUTPUT:
			{
				LL_PSBoardOutput(true);
				Delay_mS(1000);
				LL_PSBoardOutput(false);
			}
			break;
			
		case ACT_DBG_GATE_CONTROL:
			LL_WriteToGateRegister(DataTable[REG_GATE_REGISTER]);
			break;
			
		case ACT_DBG_GATE_EN:
			LL_PulseSYNC();
			break;

		default:
			return false;
	}
	
	return true;
}
//------------------------------------------

bool CONTROL_CheckGateRegisterValue()
{
	float CurrentPerLSB = 0;
	float CurrentPerBit = 0;

	if(DataTable[REG_GATE_REGISTER] < powf(2, DataTable[REG_GATE_RESOLUTION]))
	{
		CurrentPerLSB = (float)DataTable[REG_VOLTAGE_SETPOINT] / (float)DataTable[REG_RESISTANCE_PER_LSB] / 100;

		for (int i = 0; i < DataTable[REG_GATE_RESOLUTION]; i++)
		{
			CurrentPerBit = CurrentPerLSB * powf(2, i);

			if ((CurrentPerBit > DataTable[REG_MAX_CURRENT_PER_BIT]) && (DataTable[REG_GATE_REGISTER] & (1 << i)))
				return false;
		}
	}
	else
		return false;

	return true;
}
//------------------------------------------

void CONTROL_InitBatteryChargeProcess()
{
	TargetBatteryVoltage = DataTable[REG_VOLTAGE_SETPOINT];

	CONTROL_CapBatteryState = CBS_PassiveDischarge;
	CONTROL_SetDeviceState(DS_BatteryCharge);
	CONTROL_SetDeviceSubState(SS_None);
}
//------------------------------------------

void CONTROL_BatteryChargeLogic()
{
	float VoltageError, VoltageHysteresis;
	static bool VoltageReadyFlag = false;

	ActualBatteryVoltage = DataTable[REG_ACTUAL_BAT_VOLTAGE] = MEASURE_GetBatteryVoltage();
	VoltageError = ((float)TargetBatteryVoltage - ActualBatteryVoltage) / TargetBatteryVoltage * 100;
	VoltageHysteresis = (float)DataTable[REG_VOLTAGE_HYST] / 10;
	
	// Поддержание напряжения на батарее
	if((CONTROL_State == DS_BatteryCharge || CONTROL_State == DS_Ready) && (CONTROL_TimeCounter > CONTROL_PsBoardDisableTimeout))
	{
		switch(CONTROL_CapBatteryState)
		{
			case CBS_PassiveDischarge:
				VoltageReadyFlag = false;

				LL_PSBoardOutput(false);
				LL_BatteryDischarge(false);

				if(VoltageError >= VoltageHysteresis)
				{
					CONTROL_ChargeTimeout = CONTROL_TimeCounter + DataTable[REG_BAT_CHARGE_TIMEOUT];
					CONTROL_CapBatteryState = CBS_Charge;
				}
				else
				{
					if(VoltageError < -VoltageHysteresis)
						CONTROL_CapBatteryState = CBS_ActiveDischarge;
					else
						VoltageReadyFlag = true;
				}
				break;

			case CBS_ActiveDischarge:
				LL_PSBoardOutput(false);
				LL_BatteryDischarge(true);

				if(VoltageError >= -VoltageHysteresis)
					CONTROL_CapBatteryState = CBS_PassiveDischarge;
				break;

			case CBS_Charge:
				LL_BatteryDischarge(false);
				LL_PSBoardOutput(true);

				if(VoltageError <= -VoltageHysteresis)
					CONTROL_CapBatteryState = CBS_PassiveDischarge;
				break;
		}
	}

	// Условие смены состояния
	if(CONTROL_State == DS_BatteryCharge)
	{
		if(VoltageReadyFlag && CONTROL_TimeCounter > CONTROL_AfterPulseTimeout)
			CONTROL_SetDeviceState(DS_Ready);

		if(CONTROL_CapBatteryState == CBS_Charge && CONTROL_TimeCounter > CONTROL_ChargeTimeout)
			CONTROL_SwitchToFault(DF_BATTERY);
	}
}
//------------------------------------------

void CONTROL_HandleFanLogic(bool IsImpulse)
{
	static uint32_t IncrementCounter = 0;
	static uint64_t FanOnTimeout = 0;

	// Увеличение счётчика в простое
	if (!IsImpulse)
		IncrementCounter++;

	// Включение вентилятора
	if ((IncrementCounter > ((uint32_t)DataTable[REG_FAN_OPERATE_PERIOD] * 1000)) || IsImpulse)
	{
		IncrementCounter = 0;
		FanOnTimeout = CONTROL_TimeCounter + ((uint32_t)DataTable[REG_FAN_OPERATE_TIME] * 1000);
		LL_Fan(true);
	}

	// Отключение вентилятора
	if (FanOnTimeout && (CONTROL_TimeCounter > FanOnTimeout))
	{
		FanOnTimeout = 0;
		LL_Fan(false);
	}
}
//-----------------------------------------------

void CONTROL_HandleLEDLogic(bool IsImpulse)
{
	static Int64U ExternalLampCounter = 0;

	if(CONTROL_State != DS_None)
	{
		if(CONTROL_State == DS_Fault)
		{
			if(CONTROL_TimeCounter >= ExternalLampCounter)
			{
				LL_ToggleExternalLED();
				ExternalLampCounter = CONTROL_TimeCounter + TIME_FAULT_LED_BLINK;
			}
		}
		else
		{
			if(IsImpulse)
			{
				LL_ExternalLED(true);
				ExternalLampCounter = CONTROL_TimeCounter + TIME_EXT_LED_BLINK;
			}
			else
			{
				if(CONTROL_TimeCounter >= ExternalLampCounter)
					LL_ExternalLED(false);
			}
		}
	}
}
//-----------------------------------------------

void CONTROL_SwitchToFault(Int16U Reason)
{
	CONTROL_ResetHardware(true);

	CONTROL_SetDeviceSubState(SS_None);
	CONTROL_SetDeviceState(DS_Fault);
	DataTable[REG_FAULT_REASON] = Reason;
}
//------------------------------------------

void CONTROL_SwitchToProblem(Int16U Reason)
{
	CONTROL_ResetHardware(false);

	CONTROL_SetDeviceSubState(SS_None);
	CONTROL_SetDeviceState(DS_Ready);
	DataTable[REG_PROBLEM] = Reason;
	DataTable[REG_OP_RESULT] = OPRESULT_FAIL;
}
//------------------------------------------

void CONTROL_SetDeviceState(DeviceState NewState)
{
	CONTROL_State = NewState;
	DataTable[REG_DEV_STATE] = NewState;
}
//------------------------------------------

void CONTROL_SetDeviceSubState(DeviceSubState NewSubState)
{
	CONTROL_SubState = NewSubState;
	DataTable[REG_DEV_SUBSTATE] = NewSubState;
}
//------------------------------------------

DeviceSubState CONTROL_GetSubState()
{
	return CONTROL_SubState;
}
//------------------------------------------

void Delay_mS(uint32_t Delay)
{
	uint64_t Counter = (uint64_t)CONTROL_TimeCounter + Delay;
	while(Counter > CONTROL_TimeCounter)
		CONTROL_WatchDogUpdate();
}
//------------------------------------------

void CONTROL_WatchDogUpdate()
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------


void CONTROL_CurrentEmergencyStop(Int16U Reason)
{
	Impulse = false;
	LL_FlipSpiRCK();

	CONTROL_SwitchToFault(Reason);
}
//------------------------------------------

void CONTROL_HandleSynchronizationTimeout()
{
	if((CONTROL_SubState == SS_WaitingSync || CONTROL_SubState == SS_StartPulse)
			&& (CONTROL_TimeCounter > CONTROL_SynchronizationTimeout))
	{
		LL_WriteToGateRegister(0);

		if(CONTROL_SubState == SS_WaitingSync)
			CONTROL_SetDeviceSubState(SS_None);
		else
			CONTROL_SwitchToProblem(PROBLEM_SYNC_LINE);
	}
}
//------------------------------------------

void CONTROL_FinishProcess()
{
	LL_WriteToGateRegister(0);

	CONTROL_PsBoardDisableTimeout = CONTROL_TimeCounter + DataTable[REG_PS_BOARD_DISABLE_TIMEOUT];
	CONTROL_AfterPulseTimeout = CONTROL_TimeCounter + DataTable[REG_AFTER_PULSE_TIMEOUT];

	DataTable[REG_OP_RESULT] = OPRESULT_OK;
	CONTROL_InitBatteryChargeProcess();
}
//------------------------------------------
