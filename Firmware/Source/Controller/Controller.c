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


// Переменные
//
//
static Boolean CycleActive = FALSE;
Int16U CONTROL_Values_U[VALUES_x_SIZE];
Int16U CONTROL_Values_I[VALUES_x_SIZE];
Int16U CONTROL_Values_U_Counter = 0;
Int16U CONTROL_Values_I_Counter = 0;


// Функции
//
static Boolean CONTROL_DispatchAction(Int16U ActionID, pInt16U pUserError);
//


//------------------------------------------------------------------------------
void CONTROL_Init()
{
  // Переменные для конфигурации EndPoint
  Int16U EPIndexes[EP_COUNT] = {EP16_Data_U, EP16_Data_I};
  Int16U EPSized[EP_COUNT] = {VALUES_x_SIZE, VALUES_x_SIZE};
  pInt16U EPCounters[EP_COUNT] = {(pInt16U)&CONTROL_Values_U_Counter, (pInt16U)&CONTROL_Values_I_Counter};
  pInt16U EPDatas[EP_COUNT] = {CONTROL_Values_U, CONTROL_Values_I};

  // Конфигурация сервиса работы Data-table и EPROM
  EPROMServiceConfig EPROMService = {(FUNC_EPROM_WriteValues)&NFLASH_WriteDT, (FUNC_EPROM_ReadValues)&NFLASH_ReadDT};

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
  SCPCFind(&MASTER_DEVICE_CAN_Interface);
  IWDG_Control();
}
// -----------------------------------------------------------------------------

//------------------------------------------------------------------------------
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
    {
      if(DataTable[REG_DEV_STATE]==DS_PulseConfigReady)
      {
        if((DataTable[REG_DUT_TYPE]==DIODE)||(DataTable[REG_DUT_TYPE]==THYRISTOR))
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
			if(DataTable[REG_SCPC_EMULATION]==EMUL_SCPC_EN)
			{
				SetDeviceState(DS_PulseEnd);
				DEVPROFILE_ResetScopes(0);
				DEVPROFILE_ResetEPReadState();
			}
			else
			  {
				SetDeviceState(DS_PulseStart);
			  }
          }
        }
        else
        {
          //Команда заблокирована, введенное значение ударного тока не соответствует текущей конфигурации
          *pUserError = ERR_OPERATION_BLOCKED;
        }
      }
      else
      {
        //Команда заблокирована, т.к. установка не была сконфигурирована
        *pUserError = ERR_DEVICE_NOT_READY;
      }
      break;
    }
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
       SCPCFind(&MASTER_DEVICE_CAN_Interface);
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
  case ACT_SAVE_DT_TO_FLASH:            //Сохранить DataTable во Flash
    {
      DT_SaveNVPartToEPROM();
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
  if(DataTable[REG_DEV_STATE]==DS_BatteryChargeStart)
  {
    SCPC_CapChargeStart(&MASTER_DEVICE_CAN_Interface);
  }
  //

  //Если состояние конфигурации блоков, то конфигурируем их
  if(DataTable[REG_DEV_STATE]==DS_PulseConfig)
  {
    SCTU_Config(&MASTER_DEVICE_CAN_Interface);
  }
  //

  //Если состояние установки DS_PulseStart, то формируем импульс тока
  if(DataTable[REG_DEV_STATE]==DS_PulseStart)
  {
	DEVPROFILE_ResetScopes(0);
	SurgeCurrentProcess(&MASTER_DEVICE_CAN_Interface);
	DEVPROFILE_ResetEPReadState();
  }

  //Если установка перешла в состояние DS_PulseEnd, то через время TIME_CHANGE_STATE
  //установка переходит в состояние DS_BatteryChargeWait
  if(DataTable[REG_DEV_STATE]==DS_PulseEnd)
  {
    Int64U TimeOutCounter=CONTROL_TimeCounter+TIME_CHANGE_STATE;
    while(CONTROL_TimeCounter<TimeOutCounter)
    {
      DEVPROFILE_ProcessRequests();
      IWDG_Control();
    }
    SetDeviceState(DS_WaitTimeOut);
  }
  //

  //Ожидание общего таймаута работы установки
  if(DataTable[REG_DEV_STATE]==DS_WaitTimeOut)
  {
    Int64U TimeOutCounter=CONTROL_TimeCounter+WAIT_TIMEOUT_VALUE;
    while(CONTROL_TimeCounter<TimeOutCounter)
    {
      DEVPROFILE_ProcessRequests();
      IWDG_Control();
    }
    SetDeviceState(DS_Ready);
  }
  //


  //Меняем в ячейке макисмальный ток SCTU, при смене формы импульса
  if(DataTable[REG_WAVEFORM_TYPE]==WAVEFORM_SINE)
  {
    uint32_t SC_Max_Temp = SCPC_SC_SINE_MAX*DataTable[REG_TOTAL_SCPC];
    DataTable[REG_SC_MAX_L] = (uint16_t)SC_Max_Temp;
    DataTable[REG_SC_MAX_H] = (uint16_t)(SC_Max_Temp>>16);

    if(SC_Max_Temp>SCTU_SC_SINE_MAX)
    {
      DataTable[REG_SC_MAX_L] = (uint16_t)SCTU_SC_SINE_MAX;
      DataTable[REG_SC_MAX_H] = (uint16_t)(SCTU_SC_SINE_MAX>>16);
    }
  }

  if(DataTable[REG_WAVEFORM_TYPE]==WAVEFORM_TRAPEZE)
  {
    uint32_t SC_Max_Temp = SCPC_SC_TRAPEZE_MAX*SCPC_v20_Count;
    DataTable[REG_SC_MAX_L] = (uint16_t)SC_Max_Temp;
    DataTable[REG_SC_MAX_H] = (uint16_t)(SC_Max_Temp>>16);

    if(SC_Max_Temp>SCTU_SC_TRAPEZE_MAX)
    {
      DataTable[REG_SC_MAX_L] = (uint16_t)SCTU_SC_TRAPEZE_MAX;
      DataTable[REG_SC_MAX_H] = (uint16_t)(SCTU_SC_TRAPEZE_MAX>>16);
    }
  }
  //

  //Если задан ток больше максимально возможного, то заданное значение уменьшиться до возможного
  uint32_t CurrentMax = (DataTable[REG_SC_MAX_H]<<16);
  CurrentMax |= DataTable[REG_SC_MAX_L];

  uint32_t CurrentSet = (DataTable[REG_SC_VALUE_H]<<16);
  CurrentSet |= DataTable[REG_SC_VALUE_L];

  if(CurrentSet>CurrentMax)
  {
    DataTable[REG_SC_VALUE_L] = (uint16_t)CurrentMax;
    DataTable[REG_SC_VALUE_H] = (uint16_t)(CurrentMax>>16);
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
  Nid_Count=0;
	while(Nid_Count<DataTable[REG_TOTAL_SCPC])
	{
		SCPC_Read_Data(&MASTER_DEVICE_CAN_Interface, SCPC_Data[Nid_Count].Nid, true);
		if(SCPC_CheckStatus(Nid_Count, SCPC_Ready))
			Nid_Count++;
		else
		{
			//Блок SCPC не отвечает, сохраняем Fault
			SCPC_SaveData(Nid_Count, REG_SCPC_FAULT_REASON, ERR_SCPC_NOT_CHARGED);//SCPC не отвечает
			SetDeviceState(DS_Fault);
			SetDeviceFault(ERR_SCPC_NOT_CHARGED);
		}


		  DEVPROFILE_ProcessRequests();
		  IWDG_Control();
	}
  //

  //Преверяем правильно ли задан тип формы ударного тока
  if((DataTable[REG_WAVEFORM_TYPE]!=WAVEFORM_SINE)&&(DataTable[REG_WAVEFORM_TYPE]!=WAVEFORM_TRAPEZE))
  {
    SetDeviceState(DS_Fault);
    DataTable[REG_FAULT_REASON]=ERR_WAVEFORM_TYPE;
    return;
  }
  //

  //Всем блокам присваиваем значение ударного тока равным нулю и устанавливаем время пульсации
  Nid_Count=0;
  while(Nid_Count<DataTable[REG_TOTAL_SCPC])
  {
    SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, 0);
    SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_PULSE_DURATION, DataTable[REG_PULSE_DURATION]);
    SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
    if(SCPC_Data[Nid_Count].SC_PulseValue==0)
    {
      Nid_Count++;
    }
    IWDG_Control();

	DEVPROFILE_ProcessRequests();
  }
  //

  //Выбираем нужный канал измерения
  MeasureChannelSet(DataTable[REG_CHANNEL],false);
  //

  //----------Конфигурация SCHuntAmplifier-------------

  //Вычисление коэффициента усиления
  uint32_t CurrentSet = (DataTable[REG_SC_VALUE_H]<<16);
  CurrentSet |= DataTable[REG_SC_VALUE_L];

  float Ushunt_mV = ((float)DataTable[REG_R_SHUNT])/1000*CurrentSet;
  float Ky = ((float)ADC_REF_MV)/Ushunt_mV;

  if(DataTable[REG_CHANNEL]==CHANNEL_1)
  {
    Set_K_ShuntAmplifier1(Ky);
  }

  if(DataTable[REG_CHANNEL]==CHANNEL_2)
  {
    Set_K_ShuntAmplifier2(Ky);
  }
  //---------------------------------------------------

  //Конфигурируем под формирование синуса
  if(DataTable[REG_WAVEFORM_TYPE]==WAVEFORM_SINE)
  {
    SCTU_PulseSineConfig(Interface);
  }
  //

  //Конфигурируем под формирование трапеции
  if(DataTable[REG_WAVEFORM_TYPE]==WAVEFORM_TRAPEZE)
  {
    SCTU_PulseTrapezeConfig(Interface);
  }
  //
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void MeasureChannelSet(uint16_t ChannelNumber, bool ChannelInverse)
{
  if(DataTable[REG_CHANNEL]==CHANNEL_1)
  {
    if(ChannelInverse)
    {
      CH_2_SET;
    }
    else
    {
      CH_1_SET;
    }
  }

  if(DataTable[REG_CHANNEL]==CHANNEL_2)
  {
    if(ChannelInverse)
    {
      CH_1_SET;
    }
    else
    {
      CH_2_SET;
    }
  }
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
  static uint16_t Nid_Count=0;
  static float SC_Temp=0;
  static long int CurrentSet=0;
  float SC_K_Set,SC_B_Set;

  //Распределяем значение ударного тока по блокам SCPC
  CurrentSet = (DataTable[REG_SC_VALUE_H]<<16);
  CurrentSet |= DataTable[REG_SC_VALUE_L];

  //Вводим поправки к заданию тока, если значение ударного тока <= UTM_I_MAX
  SC_K_Set = 1;
  SC_B_Set = 0;

  if(CurrentSet<=UTM_I_MAX)
  {
    SC_K_Set = (float)((Int16S)DataTable[REG_K_SC_SET])/1000;
    SC_B_Set = (Int16S)DataTable[REG_B_SC_SET];
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

    if(CurrentSet>SCPC_SC_SINE_MAX)
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
  //

  if(DataTable[REG_TOTAL_SCPC]>1)
  	Nid_Count=1;//Всем учавствующим блокам, кроме 0-го прсваиваем максимальную амплитуду
  else
	Nid_Count=0;


  while(CurrentSet>0)
  {
    DEVPROFILE_ProcessRequests();

    if(CurrentSet>SCPC_SC_SINE_MAX)
    {
      SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);

      if(SCPC_Data[Nid_Count].DevState == SCPC_Ready)
      {
        SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
        SCPC_WriteData(Interface, SCPC_Data[Nid_Count].Nid, REG_SCPC_SC_PULSE_VALUE, SCPC_SC_SINE_MAX);
		SCPC_Command(Interface, SCPC_Data[Nid_Count].Nid, ACT_SCPC_SC_PULSE_CONFIG);

        //Ждем паузу, пока блок выполняет процесс согласно заданной команде
        //Delay_mS(10);
      }

      if(SCPC_Data[Nid_Count].DevState == SCPC_PulseConfigReady)
      {
        CurrentSet -= SCPC_SC_SINE_MAX;
        Nid_Count++;
      }
    }
    else //Остаток знаения ударного тока присваеваем 0-му блоку
    {
      SCPC_Read_Data(Interface, SCPC_Data[0].Nid, true);

       if(SCPC_Data[0].DevState == SCPC_Ready)
       {
          SC_Temp = SC_K_Set*CurrentSet + ((Int16S)SC_B_Set);//Калибровка погрешности задания тока
          if(SC_Temp<0)
          {
            SC_Temp=1;
          }

          SCPC_WriteData(Interface, SCPC_Data[0].Nid, REG_SCPC_WAVEFORM_TYPE, DataTable[REG_WAVEFORM_TYPE]);
          SCPC_WriteData(Interface, SCPC_Data[0].Nid, REG_SCPC_SC_PULSE_VALUE, (uint16_t)SC_Temp);
		  SCPC_Command(Interface, SCPC_Data[0].Nid, ACT_SCPC_SC_PULSE_CONFIG);

          //Ждем паузу, пока блок выполняет процесс согласно заданной команде
          //Delay_mS(10);
       }

	   //SCPC_Read_Data(Interface, SCPC_Data[0].Nid, true);

       if(SCPC_Data[0].DevState == SCPC_PulseConfigReady){ CurrentSet=0;}
    }
    IWDG_Control();
  }

  SetDeviceState(DS_PulseConfigReady);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SurgeCurrentProcess(pBCCIM_Interface Interface)
{
  uint16_t Nid_Count=0;

  if(DataTable[REG_DUT_TYPE]==THYRISTOR)
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

  //Запуск сигналов синхронизации для SCPC
  SCPC_SYNC_SIGNAL_START;

  //Задержка запуска формирования импульса для выхода
  Delay_mS(DELAY_PULSE_START);

  //Запуск сигналов синхронизации для осциллографа
  OSC_SYNC_SIGNAL_START;
  //
  UI_Dut_MeasureStart();
  //
  Delay_mS(5);
  OSC_SYNC_SIGNAL_STOP;
  Delay_mS(1);
  OSC_SYNC_SIGNAL_START;
  Delay_mS(4);

  //
  SCPC_SYNC_SIGNAL_STOP;
  DUT_CLOSE;
  //

  Delay_mS(200);

  //Считываем статусы блоков SCPC
  Nid_Count=0;
  while(Nid_Count<DataTable[REG_TOTAL_SCPC])
  {
    SCPC_Read_Data(Interface, SCPC_Data[Nid_Count].Nid, true);
    Nid_Count++;
    IWDG_Control();
  }
  //

  //Проверяем все ли блоки SCPC сформировали заданный импульс тока
  Nid_Count=0;
  while(Nid_Count<DataTable[REG_TOTAL_SCPC])
  {
    if(SCPC_Data[Nid_Count].SC_PulseValue)
    {
      if((SCPC_Data[Nid_Count].DevState!=SCPC_PulseEnd)&&(SCPC_Data[Nid_Count].DevState!=SCPC_WaitTimeOut))
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
  //

  //Utm measure
  Utm_Measure();
  //

  //
  if(DataTable[REG_DEV_STATE]!=DS_Fault)
  {
    SetDeviceState(DS_PulseEnd);
  }
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Utm_Measure()
{
  //Переменные
  static float I_Temp=0;
  static float U_Temp=0;
  static float ADC_Imax=0,Umax=0;
  static float VoltageKoef;
  uint16_t I_Offset,U_Offset;
  static float CurrentKoef;
  int ADC_SampleAddress = 0;


  uint16_t ImaxSCAdr=0;
  uint16_t AvgDiv=0;
  double DataAvg=0;
  //

  //Обнуление массива
  for(int c=0;c<ADC_SC_LENGTH;c++)
  {
    ADC_SampleCount[c] = 0;
  }
  //

  if(DataTable[REG_CHANNEL]==CHANNEL_1)
  {
	  U_Offset=DataTable[REG_U1_OFFSET];
	  I_Offset=DataTable[REG_I1_OFFSET];
  }
  else
  {
	  U_Offset=DataTable[REG_U2_OFFSET];
	  I_Offset=DataTable[REG_I2_OFFSET];

  }

  //---------------Подсчет количества одинаковых сэмплов АЦП--------------
  ADC_SC_Shift=ADC_BUF[EXTREMUM_START_POINT+1];

  for(volatile int Ecount=EXTREMUM_START_POINT;Ecount<EXTREMUM_STOP_POINT;Ecount+=2)
  {
    ADC_SampleAddress = ADC_BUF[Ecount+1]-ADC_SC_Shift;

    if(ADC_SampleAddress>=0)
    {
      ADC_SampleCount[ADC_SampleAddress]++;
    }
  }
  //--------------------------------------------------------------


  //--------Определение и усреднение амплитуды тока---------------
  ImaxSCAdr=ADC_SC_LENGTH-1;
  while((ADC_SampleCount[ImaxSCAdr]<ADC_I_SAMPLE_THRESHOLD)&&(ImaxSCAdr>0))
  {
	  ImaxSCAdr--;
  }

  for(volatile int i=0;i<ADC_I_SAMPLE_THRESHOLD;i++)
  {
    AvgDiv += ADC_SampleCount[ImaxSCAdr-i];
    DataAvg += (ImaxSCAdr+ADC_SC_Shift)*ADC_SampleCount[ImaxSCAdr-i];
  }

  ADC_Imax = DataAvg/AvgDiv;

  //Преобразование результата в Амперы и введение калибровочной поправки
  CurrentKoef = ((float)DataTable[REG_K_DUT_I]);
  CurrentKoef = CurrentKoef/DataTable[REG_R_SHUNT]/Ky_ShuntAmplifier;
  ADC_Imax=ADC_Imax*CurrentKoef - I_Offset;

  ADC_Imax = (uint32_t)(ADC_Imax*(((float)K_ShuntAmplifier)/1000) + B_ShuntAmplifier);
  DataTable[REG_DUT_I_H] = (((uint32_t)ADC_Imax)&0xffff0000)>>16;
  DataTable[REG_DUT_I_L] = ((uint32_t)ADC_Imax)&0x0000ffff;
  //


  uint32_t CurrentMax = DataTable[REG_SC_MAX_H]<<16;
  CurrentMax |= DataTable[REG_SC_MAX_L];
  uint32_t CurrentDut = DataTable[REG_DUT_I_H]<<16;
  CurrentDut |= DataTable[REG_DUT_I_L];

  if(CurrentDut>CurrentMax)
  {
    DataTable[REG_WARNING] = WARNING_I_OUT_OF_RANGE;
  }

  if(DataTable[REG_DUT_U]>VOLTAGE_MEASURE_MAX)
  {
    DataTable[REG_WARNING] = WARNING_U_OUT_OF_RANGE;
  }
  //-------------------------------------------------------------


  //--------Усреднение напряжения в точке экстремума тока--------

  uint16_t IavgStart = ADC_SC_Shift+ImaxSCAdr-ADC_I_SAMPLE_THRESHOLD+1;

  //Поиск точки экстремума тока
  uint16_t ExtremumCount = EXTREMUM_START_POINT;
  while(ADC_BUF[ExtremumCount+1]!=IavgStart)
  {
    ExtremumCount+=2;

    if(ExtremumCount>=ADC_BUFF_LENGTH)
    {
      SetDeviceState(DS_PulseEnd);
      DataTable[REG_WARNING] = WARNING_I_OUT_OF_RANGE;
      return;
    }
  }
  //

  //Значение ExtremumCount должно быть четным, чтобы значения тока и напряжения не поменялись местами
  if(ExtremumCount&0x1){ExtremumCount++;}
  //

  Umax = 0;

  for(volatile int avg_count=0;avg_count<AvgDiv;avg_count++)
  {
      Umax += ADC_BUF[ExtremumCount+avg_count*2];
  }

  Umax = Umax/AvgDiv;

  //Преобразование результата в Вольты и введение калибровочной поправки
  VoltageKoef = ((float)DataTable[REG_K_DUT_U])/1000;
  Umax=Umax*VoltageKoef - U_Offset;

  U_Temp = (Int16S)DataTable[REG_K_U_CAL];
  U_Temp = U_Temp/1000;
  DataTable[REG_DUT_U] = (uint16_t)(U_Temp*Umax  + (Int16S)DataTable[REG_B_U_CAL]);//Калибровка погрешности измерения напряжения
  //
  //--------------------------------------------------------------


  //Перемещение данных оцифрованных сигналов в свои endpoint.
  ExtremumCount += AVERAGE_POINTS/2;
  if(ExtremumCount&0x1){ExtremumCount++;}

  for(volatile int a=0;a<EP_SIZE;a++)
  {
    //Преобразуем результат в Вольты
    VoltageKoef = ((float)DataTable[REG_DUT_U] + U_Offset)/ADC_BUF[ExtremumCount];
    U_Temp = ADC_BUF[a*2]*VoltageKoef - U_Offset;
    CONTROL_Values_U[a] = (uint16_t)(U_Temp);
    //

    //Преобразуем результат в Амперы и вводим калибровочную поправку
    CurrentKoef = ((float)DataTable[REG_K_DUT_I]);
    CurrentKoef = CurrentKoef/DataTable[REG_R_SHUNT]/Ky_ShuntAmplifier;

    I_Temp = K_ShuntAmplifier;
    I_Temp = I_Temp/1000*(ADC_BUF[a*2+1]*CurrentKoef)-I_Offset;
    CONTROL_Values_I[a] = (uint16_t)((float)(I_Temp+B_ShuntAmplifier) / 10);//Калибровка погрешности измерения тока
    //

    IWDG_Control();
  }

  CONTROL_Values_U_Counter=EP_SIZE;
  CONTROL_Values_I_Counter=EP_SIZE;
  //
}
//------------------------------------------------------------------------------


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
  while(CONTROL_TimeCounter<(Counter+Delay)){IWDG_Control();}
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void IWDG_Control(void)
{
	if(BOOT_LOADER_VARIABLE != BOOT_LOADER_REQUEST)
		IWDG_Refresh();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UI_Dut_MeasureStart(void)
{
  TIM_Start(TIM15);
}
//------------------------------------------------------------------------------

// No more.
