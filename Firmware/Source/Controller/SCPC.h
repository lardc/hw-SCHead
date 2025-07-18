// -----------------------------------------
// SCPC definitions
// ----------------------------------------

#ifndef __SCPC_H
#define __SCPC_H

#include "ZwNCAN.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "stdinc.h"
#include "Global.h"
#include "BCCIMaster.h"

//Defines

//
#define ADR_SCPC0                               101                             //Адрес первого блока SCPowerCell
#define SCPC_SC_MIN_VALUE                       0                               //Минимальное значение ударного тока для блока SCPC
#define SCPC_TIME_DELAY_CHARGE                  1000                            //Время между запуском заряда конденсаторов блоков SCPC
#define SCPC_NOT_ANS_MAX_NUM                    10                              //Максимальное количество попыток отправки команд 1-му блоку
#define SCPC_SC_SINE_MAX                        3100                            //Максимальное значение ударного тока для блока SCPC при полусинусоидальной форме импульса
#define SCPC_SC_TRAPEZE_MAX                     3100                            //Максимальное значение ударного тока для блока SCPC при трапецеидальной форме импульса


//Регистры блока SCPC
#define REG_SCPC_SC_PULSE_VALUE                 64                              //Значение ударного тока блока SCPC
#define REG_SCPC_WAVEFORM_TYPE                  65                              //Тип формы импульса ударного тока
#define REG_SCPC_TRAPEZE_EDGE_TIME              66                              //Время длительности фронта трапеции, мкС
#define REG_SCPC_PULSE_DURATION					68								//Длительность импульса (в мкс)
#define REG_SCPC_PULSE_COUNT					69								//Cчётчик пропуска импульсов синхронизации
#define REG_SCPC_BAT_VOLTAGE                    96                              //Напряжение батареи, В/10
#define REG_SCPC_VERSION                        95                              //Версия блока
#define REG_SCPC_DEV_STATE                      97                              //Состояние блока
#define REG_SCPC_FAULT_REASON                   98                              //Регистр Fault
#define REG_SCPC_DISABLE_REASON                 99                              //Регистр Disable
#define REG_SCPC_WARNING                        100                             //Регистр Warning
#define REG_SCPC_PROBLEM                        101                             //Регистр Problem
//

//Команды блока SCPC
#define ACT_SCPC_DS_NONE                        1                               //Переход в состояние ожидания
#define ACT_SCPC_BAT_START_CHARGE               2                               //Команда блоку SCPowerCell на заряд батареи конденсаторов
#define ACT_SCPC_FAULT_CLEAR                    3                               //Очистка fault
#define ACT_SCPC_WARNING_CLEAR                  4                               //Очистка warning
#define ACT_SCPC_RESET_DEVICE                   5                               //Перезапуск процессора
//-----------
#define ACT_SCPC_SC_PULSE_CONFIG                100                             //Команда блоку SCPowerCell на конфигурацию значения ударного тока
#define ACT_SCPC_SC_PULSE_START                 101                             //Запуск формирования импульса ударного тока
//

//
#define EMUL_SCPC_EN							1								//Эмуляция SCPC включена
#define EMUL_SCPC_DIS							0								//Эмуляция SCPC выключена
#define SCPC_VERSION_20                         20                              //Версия 2.0 блока SCPC
#define SCPC_VERSION_11                         11                              //Версия 1.1 блока SCPC
//
#define Master_MBOX_W_16_A			1
#define Master_MBOX_R_16_A			3
#define Master_MBOX_C_A				5

//Состояния блока SCPC
typedef enum __SCPCState
{
  SCPC_None                                       = 0,                            //Блок в неопределенном состоянии
  SCPC_Fault                                      = 1,                            //Блок в состоянии Fault
  SCPC_Disabled                                   = 2,                            //Блок в состоянии Disabled
  SCPC_WaitTimeOut                                = 3,                            //Блок в ожидании таймаута между импульсами ударного тока
  SCPC_BatteryChargeWait                          = 4,                            //Блок в состоянии ожидания заряда конденсаторной батареи
  SCPC_Ready                                      = 5,                            //Блок в состоянии готовности
  SCPC_PulseConfigReady                           = 6,                            //Блок в в сконфигурированном состоянии
  SCPC_PulseStart                                 = 7,                            //Блок в состоянии формирования импульса ударного тока
  SCPC_PulseEnd                                   = 8,                            //Блок завершил формирование импульса тока
} SCPCState;
//


//Структуры
//
struct SCPC_Data_Struct
{
  Int16U        Nid;
  Int16U        SC_PulseValue;
  Int16U        BatVoltage;
  Int16U        DevState;
  Int16U        DevFault;
  Int16U        DevDisable;
  Int16U        DevWarning;
  Int16U        DevProblem;
  Int16U        Version;
  Int16U        NotAnsCounter;
};
extern struct SCPC_Data_Struct SCPC_Data[];

//Переменные
extern uint16_t SCPC_v20_Count;

//Функции
bool SCPC_CheckStatus(Int16U NodeID, Int16U Status);
void SCPC_WriteData(pBCCIM_Interface Interface, uint16_t Nid, uint16_t Address, uint16_t Data);
void SCPC_SaveData(uint16_t Nid, uint16_t Address, uint16_t Data);
void SCPC_NotAnsCounterControl(uint16_t Nid, bool CounterValue);
void SCPC_Read_Data(pBCCIM_Interface Interface, uint16_t SCPC_id, bool ErrorCtrl);
void SCPC_Command(pBCCIM_Interface Interface, uint16_t SCPC_id, uint16_t Command);
void SCPC_Save_Nid_Version(uint16_t Nid, uint16_t Version);
void SCPC_CapChargeStart(pBCCIM_Interface Interface);
void SCPCFind(pBCCIM_Interface Interface, bool ResetToNone);
bool SCPC_GetInfo(void);
Int16U SCPC_GetCalibratedIndex(uint16_t Nid);
void SCPC_ShuffleIndexArray();
Int16U SCPC_GetIndex(Int16U InitialIndex);

#endif // __SCPC_H
