﻿#include "SCPC.h"

#include "ZwNCAN.h"
#include <string.h>
#include <stdlib.h>

// Переменные
uint16_t SCPC_v20_Count = 0;

// Структуры
struct SCPC_Data_Struct SCPC_Data[SCTU_NUM_MAX];
uint16_t SCPCNidRandomize[SCTU_NUM_MAX];

// ------------------------------------------------------------------------------
bool SCPC_CheckStatus(Int16U NodeID, Int16U Status)
{
	// Если эмуляция SCPC включена или статус соответствует
	return (DataTable[REG_SCPC_EMULATION] == EMUL_SCPC_EN) || (SCPC_Data[NodeID].DevState == Status);
}
// -----------------------------------------------------------------------------

void SCPC_WriteData(pBCCIM_Interface Interface, uint16_t Nid, uint16_t Address, uint16_t Data)
{
	uint16_t Status = BCCIM_Write16(Interface, Nid, Address, Data);

	// Блок SCPC не отвечает — передаём true
	SCPC_NotAnsCounterControl(Nid, ERR_NO_ERROR != Status);
}
// -----------------------------------------------------------------------------

void SCPC_SaveData(uint16_t Nid, uint16_t Address, uint16_t Data)
{
	Int16U Nid_Count = DataTable[REG_TOTAL_SCPC] + 1;
	
	// Поиск нужного Nid
	for(int i = 0; i < DataTable[REG_TOTAL_SCPC]; i++)
	{
		if(SCPC_Data[i].Nid == Nid)
		{
			Nid_Count = i;
		}
	}
	
	if(Nid_Count > DataTable[REG_TOTAL_SCPC])
		return; // Данного Nid нет в списке
	
	// Определения соответсвующего поля для сохранения данных
	switch(Address)
	{
		case REG_SCPC_SC_PULSE_VALUE:
			{
				SCPC_Data[Nid_Count].SC_PulseValue = Data;
				break;
			}
		case REG_SCPC_BAT_VOLTAGE:
			{
				SCPC_Data[Nid_Count].BatVoltage = Data;
				break;
			}
		case REG_SCPC_DEV_STATE:
			{
				SCPC_Data[Nid_Count].DevState = Data;
				break;
			}
		case REG_SCPC_FAULT_REASON:
			{
				SCPC_Data[Nid_Count].DevFault = Data;
				break;
			}
		case REG_SCPC_DISABLE_REASON:
			{
				SCPC_Data[Nid_Count].DevDisable = Data;
				break;
			}
		case REG_SCPC_WARNING:
			{
				SCPC_Data[Nid_Count].DevWarning = Data;
				break;
			}
		case REG_SCPC_PROBLEM:
			{
				SCPC_Data[Nid_Count].DevProblem = Data;
				break;
			}
	}
}
// -----------------------------------------------------------------------------

void SCPC_NotAnsCounterControl(uint16_t Nid, bool CounterValue)
{
	Int16U Nid_Count = DataTable[REG_TOTAL_SCPC] + 1;
	
	// Поиск нужного Nid
	for(int i = 0; i < DataTable[REG_TOTAL_SCPC]; i++)
	{
		if(SCPC_Data[i].Nid == Nid)
		{
			Nid_Count = i;
		}
	}
	
	if(Nid_Count > DataTable[REG_TOTAL_SCPC])
		return; // Данного Nid нет в списке
	
	if(CounterValue)
	{
		// Инкрементируем количество непришедших ответов
		SCPC_Data[Nid_Count].NotAnsCounter++;
	}
	else
	{
		// Ответ пришел, сбрасываем счетчик
		SCPC_Data[Nid_Count].NotAnsCounter = 0;
	}
	
	if(SCPC_Data[Nid_Count].NotAnsCounter > SCPC_NOT_ANS_MAX_NUM)
	{
		// Блок SCPC не отвечает, сохраняем Fault
		SCPC_SaveData(Nid, REG_SCPC_FAULT_REASON, ERR_SCPC_NOT_ANS);  // SCPC не отвечает
		SetDeviceState(DS_Fault);
		SetDeviceFault(ERR_SCPC_NOT_ANS);
	}
}
// -----------------------------------------------------------------------------

void SCPC_Read_Data(pBCCIM_Interface Interface, uint16_t SCPC_id, bool ErrorCtrl)
{
	uint16_t Data = 0;
	
	// Считываем значение тока
	IWDG_Control();
	BCCIM_Read16(Interface, SCPC_id, REG_SCPC_SC_PULSE_VALUE, &Data);
	SCPC_SaveData(SCPC_id, REG_SCPC_SC_PULSE_VALUE, Data);
	
	// Считываем значение напряжения на конденсаторах
	IWDG_Control();
	BCCIM_Read16(Interface, SCPC_id, REG_SCPC_BAT_VOLTAGE, &Data);
	SCPC_SaveData(SCPC_id, REG_SCPC_BAT_VOLTAGE, Data);
	
	// Считываем значение статуса
	IWDG_Control();
	BCCIM_Read16(Interface, SCPC_id, REG_SCPC_DEV_STATE, &Data);
	SCPC_SaveData(SCPC_id, REG_SCPC_DEV_STATE, Data);
	
	// Считываем значение Fault
	IWDG_Control();
	BCCIM_Read16(Interface, SCPC_id, REG_SCPC_FAULT_REASON, &Data);
	SCPC_SaveData(SCPC_id, REG_SCPC_FAULT_REASON, Data);
	
	// Считываем значение Warning
	IWDG_Control();
	BCCIM_Read16(Interface, SCPC_id, REG_SCPC_WARNING, &Data);
	SCPC_SaveData(SCPC_id, REG_SCPC_WARNING, Data);
}
// -----------------------------------------------------------------------------

void SCPC_Command(pBCCIM_Interface Interface, uint16_t SCPC_id, uint16_t Command)
{
	uint16_t Status = BCCIM_Call(Interface, SCPC_id, Command);
	
	// Блок SCPC не отвечает — передаём true
	SCPC_NotAnsCounterControl(SCPC_id, ERR_NO_ERROR != Status);
}
// -----------------------------------------------------------------------------

void SCPC_Save_Nid_Version(uint16_t Nid, uint16_t Version)
{
	Int16U Count = 0;
	
	// Поиск свободной структуры
	while(SCPC_Data[Count].Nid != 0)
	{
		Count++;
		if(Count > DataTable[REG_SCTU_SCPC_NUM])
		{
			return;
		}
	}
	
	// Сохраняем Nid в свободной структуре
	SCPC_Data[Count].Nid = Nid;
	
	// Сохраняем версию блока
	SCPC_Data[Count].Version = Version;
	
	// Инкрементируем счетчик количества подключенных блоков
	DataTable[REG_TOTAL_SCPC]++;
}
// -----------------------------------------------------------------------------

void SCPC_CapChargeStart(pBCCIM_Interface Interface)
{
	uint16_t Nid_Count = 0;
	
	// Отправляем команду запуска заряда батареи с задержкой SCPC_TIME_SHIFT_CHARGE
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_BAT_START_CHARGE);
		SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
		
		if(SCPC_Data[Nid_Count].DevState != SCPC_Ready)
		{
			Int64U Delay = CONTROL_TimeCounter;
			while(CONTROL_TimeCounter < (Delay + SCPC_TIME_DELAY_CHARGE))
			{
				IWDG_Control();
				DEVPROFILE_ProcessRequests();
			}
		}
		
		if((SCPC_CheckStatus(Nid_Count, SCPC_BatteryChargeWait)) || (SCPC_CheckStatus(Nid_Count, SCPC_Ready)))
		{
			Nid_Count++;
		}
	}
	SetDeviceState(DS_BatteryChargeWait);
	
	// Ждем когда батареи всех блоков зарядятся
	Nid_Count = 0;
	while(Nid_Count < DataTable[REG_TOTAL_SCPC])
	{
		SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
		if(SCPC_CheckStatus(Nid_Count, SCPC_Ready))
		{
			Nid_Count++;
		}
		
		DEVPROFILE_ProcessRequests();
		IWDG_Control();
	}
	
	SetDeviceState(DS_Ready);
}
// -----------------------------------------------------------------------------

void SCPCFind(pBCCIM_Interface Interface, bool ResetToNone)
{
	uint16_t Nid = DataTable[REG_SCPC0_ADR] == 65535 ? ADR_SCPC0 : DataTable[REG_SCPC0_ADR];
	SCPC_v20_Count = 0;
	DataTable[REG_TOTAL_SCPC] = 0;
	// Обнуление хранилища
	memset(SCPC_Data, 0, sizeof(SCPC_Data));
	
	// Ждем 1000 мС пока запустяться все блоки SCPC
	uint64_t SCPC_WaitStart = CONTROL_TimeCounter + 1000;
	while(CONTROL_TimeCounter < SCPC_WaitStart)
	{
		IWDG_Control();
	}
	
	// Определяем nid подключенных блоков SCPC
	// Проверка на случай после очистки памяти
	int SCPC0_Adr = DataTable[REG_SCPC0_ADR] == 65535 ? 0 : DataTable[REG_SCPC0_ADR];
	DataTable[REG_SCTU_SCPC_NUM] = DataTable[REG_SCTU_SCPC_NUM] == 65535 ? 0 : DataTable[REG_SCTU_SCPC_NUM];
	while(Nid < (DataTable[REG_SCTU_SCPC_NUM] + SCPC0_Adr))
	{
		// Считываем версию блока
		uint16_t Version = 0;
		Int16U CallBack;
		CallBack = BCCIM_Read16(Interface, Nid, REG_SCPC_VERSION, &Version);
		
		// Если ответ поступил, то сохраняем текущий Nid в таблицу
		if(CallBack == ERR_NO_ERROR)
		{
			SCPC_Save_Nid_Version(Nid,Version);
			
			// Считаем количество блоков v.2.0
			if(Version == SCPC_VERSION_20)
				SCPC_v20_Count++;
		}

		Nid++;
		IWDG_Control();
	}
	
	int N_Scpc = DataTable[REG_SCTU_SCPC_NUM];
	if(DataTable[REG_TOTAL_SCPC] < N_Scpc)
	{
		DataTable[REG_WARNING] = WARNING_SCPC_NOT_FIND;
	}
	
	// Сброс массива рандомизации
	for(int i = 0; i < DataTable[REG_TOTAL_SCPC]; i++)
		SCPCNidRandomize[i] = i;

	// Переводим блоки SCPC в состояние DS_None
	if(ResetToNone)
	{
		uint8_t Nid_Count = 0;
		while(Nid_Count < DataTable[REG_TOTAL_SCPC])
		{
			SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_DS_NONE);
			SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
			if(SCPC_CheckStatus(Nid_Count, SCPC_None))
			{
				Nid_Count++;
			}
			IWDG_Control();
		}
	}
}
// -----------------------------------------------------------------------------

bool SCPC_GetInfo()
{
	Int16U Nid_Count = DataTable[REG_TOTAL_SCPC] + 1;
	
	// Поиск нужного Nid
	for(int i = 0; i < DataTable[REG_TOTAL_SCPC]; i++)
	{
		if(SCPC_Data[i].Nid == DataTable[REG_NID_SCPC_INFO])
		{
			Nid_Count = i;
		}
		IWDG_Control();
	}
	
	if(Nid_Count >= DataTable[REG_TOTAL_SCPC])
		return 0; // Данного Nid нет в списке
	
	DataTable[REG_INFO_SCPC_NID] = SCPC_Data[Nid_Count].Nid;
	DataTable[REG_INFO_SCPC_PULSE_VALUE] = SCPC_Data[Nid_Count].SC_PulseValue;
	DataTable[REG_INFO_SCPC_BAT_VOLTAGE] = SCPC_Data[Nid_Count].BatVoltage;
	DataTable[REG_INFO_SCPC_DEV_STATE] = SCPC_Data[Nid_Count].DevState;
	DataTable[REG_INFO_SCPC_DEV_FAULT] = SCPC_Data[Nid_Count].DevFault;
	DataTable[REG_INFO_SCPC_DEV_DISABLE] = SCPC_Data[Nid_Count].DevDisable;
	DataTable[REG_INFO_SCPC_DEV_WARNING] = SCPC_Data[Nid_Count].DevWarning;
	DataTable[REG_INFO_SCPC_DEV_PROBLEM] = SCPC_Data[Nid_Count].DevProblem;
	
	return 1;
}
// ------------------------------------------------------------------------------

Int16U SCPC_GetCalibratedIndex(uint16_t Nid)
{
	for(int i = 0; i < DataTable[REG_TOTAL_SCPC]; i++)
	{
		if(SCPC_Data[i].Nid == Nid)
			return i;
	}
	return 0;
}
// -----------------------------------------------------------------------------

void SCPC_ShuffleIndexArray()
{
	if(!DataTable[REG_RANDOMIZE_CELLS])
		return;

	// Алгоритм Фишера-Йетса для перемешивания массива
	for(int i = DataTable[REG_TOTAL_SCPC] - 1; i > 0; i--)
	{
		// Генерируем случайный индекс от 0 до i
		int j = rand() % (i + 1);

		// Меняем местами элементы array[i] и array[j]
		uint16_t temp = SCPCNidRandomize[i];
		SCPCNidRandomize[i] = SCPCNidRandomize[j];
		SCPCNidRandomize[j] = temp;
	}
}
// -----------------------------------------------------------------------------

Int16U SCPC_GetIndex(Int16U InitialIndex)
{
	return DataTable[REG_RANDOMIZE_CELLS] ? SCPCNidRandomize[InitialIndex] : InitialIndex;
}
// -----------------------------------------------------------------------------
