// -----------------------------------------
// Основная логика
// ----------------------------------------

// Header
#include "Controller.h"
//
// Includes
#include "SysConfig.h"
#include "DataTable.h"
#include "SCCISlave.h"
#include "DeviceProfile.h"
#include "BCCITypes.h"
#include "BCCIMaster.h"
#include "ZwTIM.h"
#include "math.h"
#include "ZwNFlash.h"
#include "ZwADC.h"
#include "Global.h"
#include "stdlib.h"

// Структура для работы с массивом результата
typedef struct __VIEntity
{
	Int16U Voltage;
	Int16U Current;
} VIEntity, *pVIEntity;

// Переменные
static Boolean CycleActive = FALSE, SeedInit = false;
Int16U CONTROL_Values_U[VALUES_x_SIZE];
Int16U CONTROL_Values_I[VALUES_x_SIZE];
Int16U CONTROL_Values_Counter = 0;
volatile Int16U CONTROL_RawCounter = 0;

// Функции
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
void NFLASH_WriteDTShifted(uint32_t EPROMAddress, uint16_t* Buffer, uint16_t BufferSize);
void CONTROL_ResetResults();
void CONTROL_FindExtremum(int PulseNumber, bool UseCurrentScale, pInt32U Voltage, pInt32U Current);
int CONTROL_SortFunction(const void *A, const void *B);

//------------------------------------------------------------------------------
void CONTROL_Init()
{
	// Переменные для конфигурации EndPoint
	Int16U EPIndexes[EP_COUNT] = {EP16_Data_U, EP16_Data_I, EP16_RawData};
	Int16U EPSized[EP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE, ADC_BUFF_LENGTH};
	pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_Values_Counter, (pInt16U)&CONTROL_Values_Counter,
			(pInt16U)&CONTROL_RawCounter};
	pInt16U EPDatas[EP_COUNT] = {CONTROL_Values_U, CONTROL_Values_I, (pInt16U)ADC_BUF};

	// Конфигурация сервиса работы DataTable и EPROM
	EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDTShifted, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};

	// Инициализация data table
	DT_Init(EPROMService, FALSE);
	DT_SaveFirmwareInfo(CAN_SLAVE_NID, CAN_MASTER_NID);

	// Сброс значений
	DEVPROFILE_ResetControlSection();

	// Инициализация device profile
	DEVPROFILE_Init(&CONTROL_DispatchAction, &CycleActive);
	DEVPROFILE_InitEPService(EPIndexes, EPSized, EPCounters, EPDatas);
	// Сброс значений
	DEVPROFILE_ResetControlSection();

	//Определение nid блоков, которые подключены
	SCPCFind(&MASTER_DEVICE_CAN_Interface, true);
	IWDG_Control();
}
// -----------------------------------------------------------------------------

void CONTROL_ResetResults()
{
	DEVPROFILE_ResetScopes(0);
	DEVPROFILE_ResetEPReadState();

	DataTable[REG_DUT_U] = 0;
	DataTable[REG_DUT_I_L] = 0;
	DataTable[REG_DUT_I_H] = 0;

	DataTable[REG_DUT_U_PULSE2] = 0;
	DataTable[REG_DUT_I_L_PULSE2] = 0;
	DataTable[REG_DUT_I_H_PULSE2] = 0;

	DataTable[REG_DUT_U_PULSE3] = 0;
	DataTable[REG_DUT_I_L_PULSE3] = 0;
	DataTable[REG_DUT_I_H_PULSE3] = 0;
}
// -----------------------------------------------------------------------------

static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError)
{
  *pUserError = ERR_NONE;

  switch(ActionID)
  {
  case ACT_SYNC_TEST:
	  {
		SCPC_SYNC_SIGNAL_START;
		Delay_mS(10);
		SCPC_SYNC_SIGNAL_STOP;
		break;
	  }
//-------------
  case ACT_DS_NONE:     //Перевод SCTU в состояние DS_NONE
    {
      SetDeviceState(DS_None);
      DataTable[REG_FAULT_REASON]=0;
      DataTable[REG_WARNING] = 0;

      //Переводим блоки SCPC в состояние DS_None
      uint8_t Nid_Count=0;
      while(Nid_Count<DataTable[REG_TOTAL_SCPC])
      {
        SCPC_Command(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_DS_NONE);
        SCPC_Read_Data(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, true);
        if(SCPC_CheckStatus(Nid_Count, SCPC_None))
        {
          Nid_Count++;
        }
        IWDG_Control();
      }
      //
      break;
    }
//-------------
  case ACT_BAT_START_CHARGE:    //Команда на старт заряда конденсаторов
    {
      if(DataTable[REG_DEV_STATE]==DS_None)
      {
		//Если эмуляция SCPC включена
		if(DataTable[REG_SCPC_EMULATION]==EMUL_SCPC_EN)
			SetDeviceState(DS_Ready);
		else
        	SetDeviceState(DS_BatteryChargeStart);
      }
      else
      {
        //Команда заблокирована, т.к. она поступала ранее
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
   case ACT_SC_PULSE_CONFIG:    //Команда на конфигурацию установки в соответствии со значением амплитуды ударного тока
    {
      if(DataTable[REG_DEV_STATE]==DS_Ready)
      {
        //Проверка на возможность конфигурации (значение тока должно находится в рабочем диапазоне)
        Int32U CurrentSet = (DataTable[REG_SC_VALUE_H]<<16);
        CurrentSet |= DataTable[REG_SC_VALUE_L];

        Int32U CurrentMax = (DataTable[REG_SC_MAX_H]<<16);
        CurrentMax |= DataTable[REG_SC_MAX_L];

        if(CurrentSet<=CurrentMax)
        {
			//Если эмуляция SCPC включена
			if(DataTable[REG_SCPC_EMULATION]==EMUL_SCPC_EN)
				SetDeviceState(DS_PulseConfigReady);
			else
          		SetDeviceState(DS_PulseConfig);
        }
        else
        {
          //Неправильное значение ударного тока
          *pUserError = ERR_OPERATION_BLOCKED;
        }
      }
      else
      {
        //Команда заблокирована, установка еще не в состоянии готовности
        *pUserError = ERR_DEVICE_NOT_READY;
      }
      break;
    }
//-------------
		case ACT_SC_PULSE_START:     //Команда запуска удароного тока
			if(DataTable[REG_DEV_STATE] == DS_PulseConfigReady)
			{
				CONTROL_ResetResults();

				if((DataTable[REG_DUT_TYPE] == DIODE) || (DataTable[REG_DUT_TYPE] == THYRISTOR))
				{
					if(SECURITY_SENSOR)
					{
						SetDeviceState(DS_Fault);
						SetDeviceFault(ERR_SECURITY);
						break;
					}
					else
					{
						//Если эмуляция SCPC включена
						SetDeviceState(DataTable[REG_SCPC_EMULATION] == EMUL_SCPC_EN ? DS_PulseEnd : DS_PulseStart);
					}
				}
				else
					//Команда заблокирована, введенное значение ударного тока не соответствует текущей конфигурации
					*pUserError = ERR_OPERATION_BLOCKED;
			}
			else
				//Команда заблокирована, т.к. установка не была сконфигурирована
				*pUserError = ERR_DEVICE_NOT_READY;
			break;
//-------------
  case ACT_SET_K_SHUNT_AMP:
    {
      if(DataTable[REG_CHANNEL]==CHANNEL_1)
      {
        switch (DataTable[REG_K_SHUNT_AMP])
        {
          case 0: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N1); break;}
          case 1: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N2); break;}
          case 2: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N3); break;}
          case 3: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N4); break;}
          case 4: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N5); break;}
        }
      }

      if(DataTable[REG_CHANNEL]==CHANNEL_2)
      {
        switch (DataTable[REG_K_SHUNT_AMP])
        {
          case 0: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N1); break;}
          case 1: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N2); break;}
          case 2: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N3); break;}
          case 3: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N4); break;}
          case 4: {ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N5); break;}
        }
      }

      if((DataTable[REG_CHANNEL]!=CHANNEL_1)&&(DataTable[REG_CHANNEL]!=CHANNEL_2))
      {
        //Канал задан не верно
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
   case ACT_SCPC_FIND:
     {
       //Определение nid блоков, которые подключены
       SCPCFind(&MASTER_DEVICE_CAN_Interface, false);
       break;
     }
//-------------
   case ACT_SCPC_GET_INFO:      //По Nid блока, выводится информация в DataTable
    {
      if(DataTable[REG_NID_SCPC_INFO])
      {
        if(!SCPC_GetInfo())
        {
          //Данного Nid нет в списке
          *pUserError = ERR_OPERATION_BLOCKED;
        }
      }
      else
      {
        //Nid требуемого блока не задан
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
  case ACT_SCPC_DATA_REG_WRITE:
    {
      if(DataTable[REG_SCPC_NID])
      {
        uint16_t NodeID = DataTable[REG_SCPC_NID];
        uint16_t RegAddress = DataTable[REG_SCPC_ADDR_REG];
        uint16_t RegData = DataTable[REG_SCPC_DATA_REG];

        SCPC_WriteData(&MASTER_DEVICE_CAN_Interface, NodeID, RegAddress, RegData);

        DataTable[REG_SCPC_NID] = 0;
      }
      else
      {
        //Nid требуемого блока не задан
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
   case ACT_SCPC_REG_READ:
    {
      if(DataTable[REG_SCPC_NID])
      {
        uint16_t NodeID = DataTable[REG_SCPC_NID];
        uint16_t RegAddress = DataTable[REG_SCPC_ADDR_REG];
        uint16_t RegData = 0;

        BCCIM_Read16(&MASTER_DEVICE_CAN_Interface, NodeID, RegAddress, &RegData);
        DataTable[REG_SCPC_DATA_REG] = RegData;

        DataTable[REG_SCPC_NID] = 0;
      }
      else
      {
        //Nid требуемого блока не задан
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
    case ACT_SCPC_COMM:
    {
      if(DataTable[REG_SCPC_NID])
      {
        uint16_t NodeID = DataTable[REG_SCPC_NID];
        uint16_t ActSCPC = DataTable[REG_SCPC_COMM];

        SCPC_Command(&MASTER_DEVICE_CAN_Interface, NodeID, ActSCPC);

        DataTable[REG_SCPC_NID] = 0;
      }
      else
      {
        //Nid требуемого блока не задан
        *pUserError = ERR_OPERATION_BLOCKED;
      }
      break;
    }
//-------------
   case ACT_FAULT_CLEAR:                //Команда на очистку Fault
    {
      SetDeviceState(DS_None);
      DataTable[REG_FAULT_REASON] = 0;

      //Переводим блоки SCPC в состояние DS_None
      uint8_t Nid_Count=0;
      while(Nid_Count<DataTable[REG_TOTAL_SCPC])
      {
        SCPC_Command(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_DS_NONE);
        SCPC_Read_Data(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, true);
        if(SCPC_CheckStatus(Nid_Count, SCPC_None))
        {
          Nid_Count++;
        }
        IWDG_Control();
      }
      //
      break;
    }
//-------------
    case ACT_WARNING_CLEAR:             //Команда на очистку Warning
      {
        DataTable[REG_WARNING]=0;
        break;
      }
//-------------
  case ACT_DT_RESET_TO_DEFAULT:         //Сброс DataTable в состояние по умолчанию
    {
      DT_ResetNVPartToDefault();
      break;
    }
//-------------
  case ACT_RESTORE_DT_FROM_FLASH:       //Восстановить DataTable из Flash
    {
      DT_RestoreNVPartFromEPROM();
      break;
    }
//-------------
  default:
    {
      return FALSE;
    }
  }
  return TRUE;
}
// -----------------------------------------------------------------------------

void CONTROL_Idle()
{
	//Зупуск заряда конденсаторов в блоках SCPC
	if(DataTable[REG_DEV_STATE] == DS_BatteryChargeStart)
	{
		SCPC_CapChargeStart(&MASTER_DEVICE_CAN_Interface);
	}
	//

	//Если состояние конфигурации блоков, то конфигурируем их
	if(DataTable[REG_DEV_STATE] == DS_PulseConfig)
	{
		SCTU_Config(&MASTER_DEVICE_CAN_Interface);
	}
	//

	//Если состояние установки DS_PulseStart, то формируем импульс тока
	if(DataTable[REG_DEV_STATE] == DS_PulseStart)
	{
		SurgeCurrentProcess(&MASTER_DEVICE_CAN_Interface);
	}

	//Если установка перешла в состояние DS_PulseEnd, то через время TIME_CHANGE_STATE
	//установка переходит в состояние DS_BatteryChargeWait
	if(DataTable[REG_DEV_STATE] == DS_PulseEnd)
	{
		Int64U TimeOutCounter = CONTROL_TimeCounter + TIME_CHANGE_STATE;
		while(CONTROL_TimeCounter < TimeOutCounter)
		{
			DEVPROFILE_ProcessRequests();
			IWDG_Control();
		}
		SetDeviceState(DS_WaitTimeOut);
	}
	//

	//Ожидание общего таймаута работы установки
	if(DataTable[REG_DEV_STATE] == DS_WaitTimeOut)
	{
		Int64U TimeOutCounter = CONTROL_TimeCounter + WAIT_TIMEOUT_VALUE;
		while(CONTROL_TimeCounter < TimeOutCounter)
		{
			DEVPROFILE_ProcessRequests();
			IWDG_Control();
		}
		SetDeviceState(DS_Ready);
	}
	//


	//Меняем в ячейке макисмальный ток SCTU, при смене формы импульса
	if(DataTable[REG_WAVEFORM_TYPE] == WAVEFORM_SINE)
	{
		uint32_t SCMaxAvailableCurrent = SCPC_SC_SINE_MAX * DataTable[REG_TOTAL_SCPC] / DataTable[REG_PULSE_COUNT];

		DataTable[REG_SC_MAX_L] = SCMaxAvailableCurrent;
		DataTable[REG_SC_MAX_H] = (SCMaxAvailableCurrent >> 16);
	}

	if(DataTable[REG_WAVEFORM_TYPE] == WAVEFORM_TRAPEZE)
	{
		uint32_t SCMaxAvailableCurrent = SCPC_SC_TRAPEZE_MAX * SCPC_v20_Count;
		DataTable[REG_SC_MAX_L] = (uint16_t)SCMaxAvailableCurrent;
		DataTable[REG_SC_MAX_H] = (uint16_t)(SCMaxAvailableCurrent >> 16);
	}
	//

	//Если задан ток больше максимально возможного, то заданное значение уменьшиться до возможного
	uint32_t CurrentMax = (Int32U)DataTable[REG_SC_MAX_H] << 16;
	CurrentMax |= DataTable[REG_SC_MAX_L];

	uint32_t CurrentSet = (Int32U)DataTable[REG_SC_VALUE_H] << 16;
	CurrentSet |= DataTable[REG_SC_VALUE_L];

	if(CurrentSet > CurrentMax)
	{
		DataTable[REG_SC_VALUE_L] = (uint16_t)CurrentMax;
		DataTable[REG_SC_VALUE_H] = (uint16_t)(CurrentMax >> 16);
		DataTable[REG_WARNING] = WARNING_SC_CUT_OFF;
	}
	//

	DEVPROFILE_ProcessRequests();

	IWDG_Control();
}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SCTU_Config(pBCCIM_Interface Interface)
{
	uint16_t Nid_Count;

	//Проверяем заряжены ли все блоки
	Nid_Count = 0;
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		SCPC_Read_Data(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, true);
		if(SCPC_CheckStatus(Nid_Count, SCPC_Ready))
			Nid_Count++;
		else
		{
			//Блок SCPC не отвечает, сохраняем Fault
			SCPC_SaveData(Nid_Count, REG_SCPC_FAULT_REASON, ERR_SCPC_NOT_CHARGED);	//SCPC не отвечает
			SetDeviceState(DS_Fault);
			SetDeviceFault(ERR_SCPC_NOT_CHARGED);
		}

		DEVPROFILE_ProcessRequests();
		IWDG_Control();
	}
	//

	//Преверяем правильно ли задан тип формы ударного тока
	if((DataTable[REG_WAVEFORM_TYPE] != WAVEFORM_SINE) && (DataTable[REG_WAVEFORM_TYPE] != WAVEFORM_TRAPEZE))
	{
		SetDeviceState(DS_Fault);
		DataTable[REG_FAULT_REASON] = ERR_WAVEFORM_TYPE;
		return;
	}
	//

	//Всем блокам присваиваем значение ударного тока равным нулю и устанавливаем время пульсации
	Nid_Count = 0;
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, 0);
		SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_PULSE_DURATION, DataTable[REG_PULSE_DURATION]);
		SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
		if(SCPC_Data[Nid_Count].SC_PulseValue == 0)
		{
			Nid_Count++;
		}
		IWDG_Control();

		DEVPROFILE_ProcessRequests();
	}
	//

	//Выбираем нужный канал измерения
	MeasureChannelSet(DataTable[REG_CHANNEL], false);
	//

	//----------Конфигурация SCHuntAmplifier-------------

	//Вычисление коэффициента усиления
	uint32_t CurrentSet = (Int32U)DataTable[REG_SC_VALUE_H] << 16;
	CurrentSet |= DataTable[REG_SC_VALUE_L];

	float Ushunt_mV = ((float)DataTable[REG_R_SHUNT]) / 1000 * CurrentSet;
	float Ky = ((float)ADC_REF_MV) / Ushunt_mV;

	if(DataTable[REG_CHANNEL] == CHANNEL_1)
	{
		Set_K_ShuntAmplifier1(Ky);
	}

	if(DataTable[REG_CHANNEL] == CHANNEL_2)
	{
		Set_K_ShuntAmplifier2(Ky);
	}
	//---------------------------------------------------

	//Конфигурируем под формирование синуса
	if(DataTable[REG_WAVEFORM_TYPE] == WAVEFORM_SINE)
	{
		SCTU_PulseSineConfig(Interface);
	}
	//

	//Конфигурируем под формирование трапеции
	if(DataTable[REG_WAVEFORM_TYPE] == WAVEFORM_TRAPEZE)
	{
		SCTU_PulseTrapezeConfig(Interface);
	}
	//
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void MeasureChannelSet(uint16_t ChannelNumber, bool ChannelInverse)
{
	if(DataTable[REG_CHANNEL] == CHANNEL_1)
		ChannelInverse ? CH_2_SET : CH_1_SET;

	if(DataTable[REG_CHANNEL] == CHANNEL_2)
		ChannelInverse ? CH_1_SET : CH_2_SET;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SCTU_PulseTrapezeConfig(pBCCIM_Interface Interface)
{
  uint16_t Nid_Count=0;
  float SC_Temp=0;
  long int CurrentSet=0;
  float SC_K_Set, SC_B_Set;


  CurrentSet = (DataTable[REG_SC_VALUE_H]<<16);
  CurrentSet |= DataTable[REG_SC_VALUE_L];

  //Вводим поправки к заданию тока, если значение ударного тока <= UTM_I_MAX
  SC_K_Set = 1;
  SC_B_Set = 0;

  if(CurrentSet<=UTM_I_MAX)
  {
    //SC_K_Set = ((float)DataTable[REG_K_SC_SET])/1000;
    //SC_B_Set = DataTable[REG_B_SC_SET];
  }
  //


  //Проверяем не было ли указания сконфигурировать только один блок
  if(DataTable[REG_NID_SCPC_CONFIG]!=0)
  {
    Nid_Count=0;

    //Проверяем есть ли требуемый блок в списке подключенных
    while(SCPC_Data[Nid_Count].Nid != DataTable[REG_NID_SCPC_CONFIG])
    {
      Nid_Count++;

      if(Nid_Count>=DataTable[REG_SCTU_SCPC_NUM])
      {
        DataTable[REG_WARNING] = WARNING_NID_NOT_FOUND;
        SetDeviceState(DS_Ready);
        DataTable[REG_NID_SCPC_CONFIG] = 0;
        return;
      }
    }
    //

    //Проверяем правильно ли выбран блок для трапецеидального импульса
    if(SCPC_Data[Nid_Count].Version != SCPC_VERSION_20)
    {
        DataTable[REG_WARNING] = WARNING_WRONG_VERSION;
        SetDeviceState(DS_Ready);
        DataTable[REG_NID_SCPC_CONFIG] = 0;
        return;
    }
    //

    if(CurrentSet>SCPC_SC_TRAPEZE_MAX)
    {
      CurrentSet = SCPC_SC_TRAPEZE_MAX;
    }

    SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);

    if(SCPC_Data[Nid_Count].DevState == SCPC_Ready)
    {
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_TRAPEZE_EDGE_TIME, DataTable[REG_TRAPEZE_EDGE_TIME]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, CurrentSet);
      SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_SC_PULSE_CONFIG);
    }

    DataTable[REG_NID_SCPC_CONFIG] = 0;

    SetDeviceState(DS_PulseConfigReady);
    return;
  }
  //


//---------------Распределяем значение ударного тока по блокам SCPC-------------

  //Поиск младшего блока v.2.0
  uint16_t Nid_LowSCPC_v20=0;
  Nid_Count=0;

  while(SCPC_Data[Nid_Count].Version!=SCPC_VERSION_20)
  {
    Nid_Count++;
  }
  Nid_LowSCPC_v20 = Nid_Count;

  Nid_Count++;

  //Поиск следующего блока
  while(SCPC_Data[Nid_Count].Version!=SCPC_VERSION_20)
  {
    Nid_Count++;
  }
  //

  while(CurrentSet>0)
  {
    if(CurrentSet>SCPC_SC_TRAPEZE_MAX)
    {
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_TRAPEZE_EDGE_TIME, DataTable[REG_TRAPEZE_EDGE_TIME]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, SCPC_SC_TRAPEZE_MAX);
      SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_SC_PULSE_CONFIG);
      SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);

      if(SCPC_Data[Nid_Count].DevState == SCPC_PulseConfigReady)
      {
        CurrentSet -= SCPC_SC_TRAPEZE_MAX;

        //Поиск следующего блока SCPC v.2.0
        Nid_Count++;
        while(SCPC_Data[Nid_Count].Version!=SCPC_VERSION_20)
        {
          if(Nid_Count>=DataTable[REG_TOTAL_SCPC])
          {
            break;
          }
          Nid_Count++;
        }
      }
    }
    else //Остаток знаения ударного тока присваеваем младшему блоку SCPC v.2.0
    {
      SC_Temp = SC_K_Set*CurrentSet;
      CurrentSet = ((uint32_t)SC_Temp)-((uint32_t)SC_B_Set);//Калибровка погрешности задания тока
      if(CurrentSet<0)
      {
        CurrentSet=0;
      }

      SCPC_WriteData(Interface, SCPC_Data[Nid_LowSCPC_v20].Nid, REG_SCPC_TRAPEZE_EDGE_TIME, DataTable[REG_TRAPEZE_EDGE_TIME]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_LowSCPC_v20].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
      SCPC_WriteData(Interface, SCPC_Data[Nid_LowSCPC_v20].Nid, REG_SCPC_SC_PULSE_VALUE, (uint32_t)CurrentSet);
      SCPC_Command(Interface, SCPC_Data[Nid_LowSCPC_v20].Nid, ACT_SCPC_SC_PULSE_CONFIG);
      SCPC_Read_Data(Interface, SCPC_Data[Nid_LowSCPC_v20].Nid, true);

      if(SCPC_Data[Nid_LowSCPC_v20].DevState == DS_PulseConfigReady){ CurrentSet=0;}

    }
    IWDG_Control();
  }

  SetDeviceState(DS_PulseConfigReady);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SCTU_PulseSineConfig(pBCCIM_Interface Interface)
{
	Int16U PulseCount = DataTable[REG_PULSE_COUNT], CalibratedNID = 0, Nid_Count = 0;
	Int32U CurrentSet = 0, CurrentSetTemp = 0;

	//Распределяем значение ударного тока по блокам SCPC
	CurrentSet = (Int32U)DataTable[REG_SC_VALUE_H] << 16;
	CurrentSet |= DataTable[REG_SC_VALUE_L];
	CurrentSetTemp = CurrentSet; // Сохраняем значение для цикла, если импульсов >1

	//Вводим поправки к заданию тока, если значение ударного тока <= UTM_I_MAX
	float SC_K_Set = 1.0f;
	float SC_B_Set = 0.0f;
	if(CurrentSet <= UTM_I_MAX)
	{
		SC_K_Set = (float)((Int16S)DataTable[REG_K_SC_SET]) / 1000;
		SC_B_Set = (Int16S)DataTable[REG_B_SC_SET];
	}

	//Проверяем не было ли указания сконфигурировать только один блок
	if(DataTable[REG_NID_SCPC_CONFIG]!=0)
	{
		Nid_Count=0;

		//Проверяем есть ли требуемый блок в списке подключенных
		while(SCPC_Data[Nid_Count].Nid != DataTable[REG_NID_SCPC_CONFIG])
		{
			Nid_Count++;

			if(Nid_Count >= DataTable[REG_SCTU_SCPC_NUM])
			{
				DataTable[REG_WARNING] = WARNING_NID_NOT_FOUND;
				SetDeviceState(DS_Ready);
				DataTable[REG_NID_SCPC_CONFIG] = 0;
				return;
			}
		}

		if(CurrentSet > SCPC_SC_SINE_MAX)
		{
			CurrentSet = SCPC_SC_SINE_MAX;
		}

		SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);

		if(SCPC_Data[Nid_Count].DevState == SCPC_Ready)
		{
			SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
			SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, CurrentSet);
			SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_SC_PULSE_CONFIG);
		}

		DataTable[REG_NID_SCPC_CONFIG] = 0;

		SetDeviceState(DS_PulseConfigReady);
		return;
	}

	Int16U Pulse1CalibratedIndex = 0;
	Int16U Pulse2CalibratedIndex = SCPC_GetCalibratedIndex(DataTable[REG_SCPC_NID_SECOND_GROUP]);
	Int16U Pulse3CalibratedIndex = SCPC_GetCalibratedIndex(DataTable[REG_SCPC_NID_THIRD_GROUP]);

	// Рандомизация массива индексов
	if(!SeedInit)
	{
		srand(CONTROL_TimeCounter & 0xFFFFFFFF);
		SeedInit = true;
	}
	SCPC_ShuffleIndexArray();

	Nid_Count = 0;
	while(PulseCount > 0)
	{
		// Определение индекса откалиброванного блока для текущей итерации
		switch(PulseCount)
		{
			case 3:
				CalibratedNID = Pulse3CalibratedIndex;
				break;
			case 2:
				CalibratedNID = Pulse2CalibratedIndex;
				break;
			default:
				CalibratedNID = Pulse1CalibratedIndex;
				break;
		}

		while(CurrentSet > 0 && Nid_Count < DataTable[REG_SCTU_SCPC_NUM])
		{
			DEVPROFILE_ProcessRequests();
			Int16U CellIndex = SCPC_GetIndex(Nid_Count);

			// Условие пропуска блоков в зависимости от итогового количества импульсов
			switch(DataTable[REG_PULSE_COUNT])
			{
				case 1:
					if(CellIndex == Pulse1CalibratedIndex)
					{
						Nid_Count++;
						continue;
					}
					break;

				case 2:
					if(CellIndex == Pulse1CalibratedIndex || CellIndex == Pulse2CalibratedIndex)
					{
						Nid_Count++;
						continue;
					}
					break;

				case 3:
					if(CellIndex == Pulse1CalibratedIndex || CellIndex == Pulse2CalibratedIndex
							|| CellIndex == Pulse3CalibratedIndex)
					{
						Nid_Count++;
						continue;
					}
					break;
			}

			if(CurrentSet > SCPC_SC_SINE_MAX)
			{
				SCPC_Read_Data(Interface, SCPC_Data[CellIndex].Nid, true);

				if(SCPC_Data[CellIndex].DevState == SCPC_Ready)
				{
					SCPC_WriteData(Interface, SCPC_Data[CellIndex].Nid, REG_SCPC_WAVEFORM_TYPE,
							DataTable[REG_WAVEFORM_TYPE]);
					SCPC_WriteData(Interface, SCPC_Data[CellIndex].Nid, REG_SCPC_SC_PULSE_VALUE, SCPC_SC_SINE_MAX);
					SCPC_WriteData(Interface, SCPC_Data[CellIndex].Nid, REG_SCPC_PULSE_COUNT, PulseCount);
					SCPC_Command(Interface, SCPC_Data[CellIndex].Nid, ACT_SCPC_SC_PULSE_CONFIG);

					//Ждем паузу, пока блок выполняет процесс согласно заданной команде
					//Delay_mS(10);
				}

				if(SCPC_Data[CellIndex].DevState == SCPC_PulseConfigReady)
				{
					CurrentSet -= SCPC_SC_SINE_MAX;
					Nid_Count++;
				}
			}
			else //Остаток знаения ударного тока присваеваем откалиброванному блоку
			{

				SCPC_Read_Data(Interface, SCPC_Data[CalibratedNID].Nid, true);

				if(SCPC_Data[CalibratedNID].DevState == SCPC_Ready)
				{
					float SC_Temp = SC_K_Set * CurrentSet + SC_B_Set; //Калибровка погрешности задания тока
					SC_Temp = (SC_Temp < 1) ? 1 : SC_Temp;

					SCPC_WriteData(Interface, SCPC_Data[CalibratedNID].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
					SCPC_WriteData(Interface, SCPC_Data[CalibratedNID].Nid, REG_SCPC_SC_PULSE_VALUE, (uint16_t)SC_Temp);
					SCPC_WriteData(Interface, SCPC_Data[CalibratedNID].Nid, REG_SCPC_PULSE_COUNT, PulseCount);
					SCPC_Command(Interface, SCPC_Data[CalibratedNID].Nid, ACT_SCPC_SC_PULSE_CONFIG);

					//Ждем паузу, пока блок выполняет процесс согласно заданной команде
					//Delay_mS(10);
				}

				//SCPC_Read_Data(Interface, SCPC_Data[CalibratedNID].Nid, true);

				if(SCPC_Data[CalibratedNID].DevState == SCPC_PulseConfigReady)
				{
					CurrentSet = 0;
				}
			}
			IWDG_Control();
		}

		PulseCount--;
		CurrentSet = CurrentSetTemp;
	}

  SetDeviceState(DS_PulseConfigReady);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SurgeCurrentProcess(pBCCIM_Interface Interface)
{
	uint16_t Nid_Count = 0;
	Int16U PulseCount = 0;

	if(DataTable[REG_DUT_TYPE] == THYRISTOR)
	{
		//Открытие тиристора
		DUT_OPEN;
		Delay_mS(5);

		//Проверка на окрытие тиристора
		if(CURRENT_DUT_CONTROL)
		{
			DUT_CLOSE;
			SetDeviceState(DS_Fault);
			SetDeviceFault(ERR_DUT_IS_NOT_OPEN);
			return;
		}
	}
	Delay_mS(200);

	// Полный цикл формирования состоит из:
	// 1. Фиксированная задержка SCPC_PREPULSE_PAUSE перед импульсом
	// 2. Длительность импульса
	// 3. Фиксированная минимальная 1мс между импульсами
	// 4. Переменная задержка между импульсами

	Int16U PulseDelay = DataTable[REG_PULSE_DURATION] / 1000;
	Int16U PauseDelay = DataTable[REG_PAUSE_DURATION] / 1000 - SCPC_PREPULSE_PAUSE;

	while(PulseCount < DataTable[REG_PULSE_COUNT])
	{
		//Запуск сигналов синхронизации для SCPC
		SCPC_SYNC_SIGNAL_START;
		OSC_SYNC_SIGNAL_START;

		Delay_mS(SCPC_PREPULSE_PAUSE);

		// Оцифровка запускается перед первым импульсом
		if(PulseCount == 0)
			UI_Dut_MeasureStart();

		Delay_mS(PulseDelay);

		SCPC_SYNC_SIGNAL_STOP;
		OSC_SYNC_SIGNAL_STOP;

		Delay_mS(PauseDelay);
		PulseCount++;
	}
	DUT_CLOSE;
	Delay_mS(200);

	//Считываем статусы блоков SCPC
	Nid_Count = 0;
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
		Nid_Count++;
		IWDG_Control();
	}

	//Проверяем все ли блоки SCPC сформировали заданный импульс тока
	Nid_Count = 0;
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		if(SCPC_Data[Nid_Count].SC_PulseValue)
		{
			if((SCPC_Data[Nid_Count].DevState != SCPC_PulseEnd) && (SCPC_Data[Nid_Count].DevState != SCPC_WaitTimeOut))
			{
				//Один из блоков SCPC не сформировал импульс тока, сохраняем Fault
				SCPC_SaveData(SCPC_Data[Nid_Count].Nid, REG_SCPC_FAULT_REASON, ERR_SCPC_PULSE);
				SetDeviceState(DS_Fault);
				SetDeviceFault(ERR_SCPC_PULSE);
			}
		}
		Nid_Count++;
		IWDG_Control();
	}

	//Utm measure
	Utm_Measure();

	if(DataTable[REG_DEV_STATE] != DS_Fault)
		SetDeviceState(DS_PulseEnd);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Utm_Measure()
{
	// Коэффициенты пересчёта
	float U_K = DataTable[REG_K_DUT_U] * 0.001f;
	float I_K = (float)DataTable[REG_K_DUT_I] / DataTable[REG_R_SHUNT] / Ky_ShuntAmplifier;

	float U_Offset, I_Offset;
	if(DataTable[REG_CHANNEL] == CHANNEL_1)
	{
		U_Offset = DataTable[REG_U1_OFFSET];
		I_Offset = DataTable[REG_I1_OFFSET];
	}
	else
	{
		U_Offset = DataTable[REG_U2_OFFSET];
		I_Offset = DataTable[REG_I2_OFFSET];
	}

	// Коэффициенты тонкой подстройки напряжения
	float U_Kfine = DataTable[REG_K_U_CAL] * 0.001f;
	float U_Bfine = (Int16S)DataTable[REG_B_U_CAL];

	// Расчёт размера актуального окна
	// Число пауз и число импульсов с учётом ширины
	Int16U ActualDataSize = ((Int32U)DataTable[REG_PULSE_DURATION] * DataTable[REG_PULSE_COUNT]
			+ (Int32U)DataTable[REG_PAUSE_DURATION] * (DataTable[REG_PULSE_COUNT] - 1)) / TIMER15_uS + SAMPLING_TAIL;
	ActualDataSize = (ActualDataSize > VALUES_x_SIZE) ? VALUES_x_SIZE : ActualDataSize;

	// Определение необходимости масштабировать ток
	Int32U CurrentSet = ((Int32U)DataTable[REG_SC_VALUE_H] << 16) | DataTable[REG_SC_VALUE_L];
	bool UseCurrentScale = CurrentSet > SCALE_CURRENT_VALUE;

	// Пересчёт значений в исходном массиве и копирование в EP
	pVIEntity rawVI = (pVIEntity)ADC_BUF;
	for(int i = 0; i < ActualDataSize; i++)
	{
		// Напряжение
		float U = U_K * rawVI[i].Voltage - U_Offset;
		U = U * U_Kfine + U_Bfine;

		rawVI[i].Voltage = U > 0 ? (Int16U)U : 0;
		CONTROL_Values_U[i] = rawVI[i].Voltage;

		// Ток
		float I = K_ShuntAmplifier * I_K * rawVI[i].Current - I_Offset;
		Int32U Iint = I > 0 ? (Int32U)I : 0;

		if(UseCurrentScale)
		{
			// В исходный массив сохраняем со сдвигом на 1бит, чтобы исключить переполнение
			rawVI[i].Current = Iint >> 1;

			// а в EP деление на 10
			CONTROL_Values_I[i] = Iint / 10;
		}
		else
		{
			rawVI[i].Current = Iint;
			CONTROL_Values_I[i] = Iint;
		}
	}
	CONTROL_Values_Counter = ActualDataSize;

	// Определение экстремумов тока и расчёт напряжения для всех импульсов
	for(int i = 1; i <= DataTable[REG_PULSE_COUNT]; i++)
	{
		Int32U Voltage = 0, Current = 0;
		CONTROL_FindExtremum(i, UseCurrentScale, &Voltage, &Current);

		// Сохранение результата
		Int16U CurrentLow = Current & 0xFFFF;
		Int16U CurrentHigh = Current >> 16;

		switch(i)
		{
			case 1:
				DataTable[REG_DUT_U] = Voltage;
				DataTable[REG_DUT_I_L] = CurrentLow;
				DataTable[REG_DUT_I_H] = CurrentHigh;
				break;

			case 2:
				DataTable[REG_DUT_U_PULSE2] = Voltage;
				DataTable[REG_DUT_I_L_PULSE2] = CurrentLow;
				DataTable[REG_DUT_I_H_PULSE2] = CurrentHigh;
				break;

			case 3:
				DataTable[REG_DUT_U_PULSE3] = Voltage;
				DataTable[REG_DUT_I_L_PULSE3] = CurrentLow;
				DataTable[REG_DUT_I_H_PULSE3] = CurrentHigh;
				break;
		}
	}
}
//------------------------------------------------------------------------------

void CONTROL_FindExtremum(int PulseNumber, bool UseCurrentScale, pInt32U Voltage, pInt32U Current)
{
	pVIEntity rawVI = (pVIEntity)ADC_BUF;

	// Определение границ импульса
	Int16U StartIndex = (PulseNumber - 1) * (DataTable[REG_PULSE_DURATION] + DataTable[REG_PAUSE_DURATION]) / TIMER15_uS;
	Int16U EndIndex = StartIndex + DataTable[REG_PULSE_DURATION] / TIMER15_uS;

	// Поиск экстремума сортировкой
	qsort((void *)(rawVI + StartIndex), EndIndex - StartIndex, sizeof(VIEntity), CONTROL_SortFunction);

	// Усреднение значений на экстремуме
	float AvgVoltage = 0, AvgCurrent = 0;
	for(int i = 0; i < EXTREMUM_POINTS; i++)
	{
		AvgVoltage += rawVI[StartIndex + i].Voltage;
		AvgCurrent += rawVI[StartIndex + i].Current;
	}
	*Voltage = AvgVoltage / EXTREMUM_POINTS;
	*Current = AvgCurrent * (UseCurrentScale ? 2 : 1) / EXTREMUM_POINTS;
}
//------------------------------------------------------------------------------

int CONTROL_SortFunction(const void *A, const void *B)
{
	return (int)(((pVIEntity)B)->Current) - (int)(((pVIEntity)A)->Current);
}
//------------------------------------------------------------------------------

void SetDeviceFault(Int16U Fault)
{
	DataTable[REG_FAULT_REASON] = Fault;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SetDeviceState(DeviceState NewState)
{
  DataTable[REG_DEV_STATE] = NewState;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Delay_mS(uint64_t Delay)
{
	TIM_Reset(TIM3);
	uint64_t Counter = CONTROL_TimeCounter;
	while(CONTROL_TimeCounter < (Counter + Delay))
	{
		IWDG_Control();
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void IWDG_Control(void)
{
	uint32_t McuResetFlag = (*(__IO uint32_t*)ADDRESS_FLAG_REGISTER) & 0xFFFF;

	if((McuResetFlag != FLAG_RESET_FOR_PROG) && (McuResetFlag != FLAG_RESET))
	{
		IWDG_Refresh();
	}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_Dut_MeasureStart(void)
{
	TIM_Start(TIM15);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void NFLASH_WriteDTShifted(uint32_t EPROMAddress, uint16_t* Buffer, uint16_t BufferSize)
{
	// Prepare flash
	NFLASH_Unlock();
	NFLASH_ErasePages(EPROMAddress, EPROMAddress + FLASH_PAGE_SIZE);

	// Запись данных сдвинута на 2 байта для сохранения в памяти значения флага
	NFLASH_WriteArray16(EPROMAddress + 2, Buffer, BufferSize);

	// Выставление флага загрузки прошивки после стирания
	uint16_t Temp = FLAG_LOAD_MAIN;
	NFLASH_WriteArray16(ADDRESS_FLASH_START_MCU, &Temp, 1);
}
//------------------------------------------------------------------------------

// No more.
