#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

//Команды
//
#define ACT_DS_NONE                             1                               //Переход в состояние ожидания
#define ACT_BAT_START_CHARGE                    2                               //Команда блокам SCPowerCell на заряд батареи конденсаторов
#define ACT_FAULT_CLEAR                         3                               //Очистка fault
#define ACT_WARNING_CLEAR                       4                               //Очистка warning
#define ACT_RESET_DEVICE                        5                               //Перезапуск процессора
//-----------
#define ACT_SC_PULSE_CONFIG                     100                             //Конфигурация значения ударного тока
#define ACT_SC_PULSE_START                      101                             //Запуск формирования импульса ударного тока
#define ACT_SCPC_GET_INFO                       102                             //Получить информацию о конкретном блоке SCPC
#define ACT_SCPC_FIND                           103                             //Команда на поиск блоков SCPC
#define ACT_SET_K_SHUNT_AMP                     104                             //Команда на установку заданного усиления SCShuntAmplifier
#define ACT_SCPC_DATA_REG_WRITE                 105                             //Команда для записи в регистр блока SCPC
#define ACT_SCPC_COMM                           106                             //Команда для вызова определенной команды в блоке SCPC
#define ACT_SCPC_REG_READ                       107                             //Команда для чтения регистра SCPC
#define ACT_SYNC_TEST				108
//-----------
#define ACT_RESET_FOR_PROGRAMMING               320                             //Перезапуск процессора с целью перепрограммирования
//-----------
#define ACT_SAVE_DT_TO_FLASH                    200                             //Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_DT_FROM_FLASH               201                             //Восстановление данных из FLASH
#define ACT_DT_RESET_TO_DEFAULT                 202                             //Сброс DataTable в состояние по умолчанию
//

// Регистры
// Сохраняемые регистры
#define REG_K_DUT_U                             0                               //Пропорциональный коэффициент по напряжению
#define REG_K_DUT_I                             1                               //Пропорциональный коэффициент по току
#define REG_U1_OFFSET                           2                               //Компенсация смещения нуля по напряжению канал 1
#define REG_U2_OFFSET                           3                               //Компенсация смещения нуля по напряжению канал 2
#define REG_I1_OFFSET                           4                               //Компенсация смещения нуля по току канал 1
#define REG_I2_OFFSET                           5                               //Компенсация смещения нуля по току канал 2
#define REG_K_SC_SET                            6                               //Множитель калибровочного уравнения значения задания тока
#define REG_B_SC_SET                            7                               //Смещение калибровочного уравнения значения задания тока
#define REG_K_U_CAL                             8                               //Множитель калибровочного уравнения измерения напряжения
#define REG_B_U_CAL                             9                               //Смещение калибровочного уравнения измерения напряжения
#define REG_CS2_K1_I_CAL                        10                              //Множитель калибровочного уравнения измерения тока, диапазон 1 (ЗУ 2 - автомат-ое)
#define REG_CS2_B1_I_CAL                        11                              //Смещение калибровочного уравнения измерения тока, диапазон 1 (ЗУ 2 - автомат-ое)
#define REG_CS2_K2_I_CAL                        12                              //Множитель калибровочного уравнения измерения тока, диапазон 2 (ЗУ 2 - автомат-ое)
#define REG_CS2_B2_I_CAL                        13                              //Смещение калибровочного уравнения измерения тока, диапазон 2 (ЗУ 2 - автомат-ое)
#define REG_CS2_K3_I_CAL                        14                              //Множитель калибровочного уравнения измерения тока, диапазон 3 (ЗУ 2 - автомат-ое)
#define REG_CS2_B3_I_CAL                        15                              //Смещение калибровочного уравнения измерения тока, диапазон 3 (ЗУ 2 - автомат-ое)
#define REG_CS2_K4_I_CAL                        16                              //Множитель калибровочного уравнения измерения тока, диапазон 4 (ЗУ 2 - автомат-ое) (не используется)
#define REG_CS2_B4_I_CAL                        17                              //Смещение калибровочного уравнения измерения тока, диапазон 4  (ЗУ 2 - автомат-ое) (не используется)
#define REG_CS2_K5_I_CAL                        18                              //Множитель калибровочного уравнения измерения тока, диапазон 5 (ЗУ 2 - автомат-ое) (не используется)
#define REG_CS2_B5_I_CAL                        19                              //Смещение калибровочного уравнения измерения тока, диапазон 5  (ЗУ 2 - автомат-ое) (не используется)
#define REG_SCTU_SCPC_NUM						20				//Количество блоков подключенных к установке
#define REG_SCPC0_ADR							21				//Nid первого блока SCPC
//
#define REG_CS1_K1_I_CAL                        22                              //Множитель калибровочного уравнения измерения тока, диапазон 1 (ЗУ 1 - ручное)
#define REG_CS1_B1_I_CAL                        23                              //Смещение калибровочного уравнения измерения тока, диапазон 1 (ЗУ 1 - ручное)
#define REG_CS1_K2_I_CAL                        24                              //Множитель калибровочного уравнения измерения тока, диапазон 2 (ЗУ 1 - ручное)
#define REG_CS1_B2_I_CAL                        25                              //Смещение калибровочного уравнения измерения тока, диапазон 2 (ЗУ 1 - ручное)
#define REG_CS1_K3_I_CAL                        26                              //Множитель калибровочного уравнения измерения тока, диапазон 3 (ЗУ 1 - ручное)
#define REG_CS1_B3_I_CAL                        27                              //Смещение калибровочного уравнения измерения тока, диапазон 3 (ЗУ 1 - ручное)
#define REG_CS1_K4_I_CAL                        28                              //Множитель калибровочного уравнения измерения тока, диапазон 4 (ЗУ 1 - ручное) (не используется)
#define REG_CS1_B4_I_CAL                        29                              //Смещение калибровочного уравнения измерения тока, диапазон 4  (ЗУ 1 - ручное) (не используется)
#define REG_CS1_K5_I_CAL                        30                              //Множитель калибровочного уравнения измерения тока, диапазон 5 (ЗУ 1 - ручное) (не используется)
#define REG_CS1_B5_I_CAL                        31                              //Смещение калибровочного уравнения измерения тока, диапазон 5  (ЗУ 1 - ручное) (не используется)

#define REG_SCPC_NID_SECOND_GROUP				40								//Регистр NID откалиброванного блока SCPC из второй группы по созданию импульса
#define REG_SCPC_NID_THIRD_GROUP				41								//Регистр NID откалиброванного блока SCPC из третьей группы по созданию импульса

// Несохраняемы регистры чтения-записи
#define REG_SC_VALUE_L                          64                              //Значение ударного тока, А (младшая часть)
#define REG_SC_VALUE_H                          65                              //Значение ударного тока, А (старшая часть)
#define REG_DUT_TYPE                            66                              //Тип испытуемого прибора (1234-DIODE, 5678-THYRISTOR)
#define REG_R_SHUNT                             67                              //Сопротивление шунта в мкОм
#define REG_NID_SCPC_CONFIG                     68                              //Не используется
#define REG_NID_SCPC_INFO                       69                              //Nid блока SCPC, информацию о котором необходимо получить
#define REG_WAVEFORM_TYPE                       70                              //Задание формы ударного тока (полусинус/трапеция)
#define REG_TRAPEZE_EDGE_TIME                   71                              //Время длительности фронта трапеции, мкС
#define REG_CHANNEL                             72                              //Регистр выбора канала измерения
#define REG_K_SHUNT_AMP                         73                              //Задаваемое значение усиления SCShuntAmplifier
//
#define REG_SCPC_DATA_REG                       74                              //Регистр данных, которые надо записать в регистр определенного блока SCPC
#define REG_SCPC_ADDR_REG                       75                              //Регистр адреса регистра блока SCPC, в который надо записать данные
#define REG_SCPC_COMM                           76                              //Регистр команды, которую надо вызвать в блоке SCPC
#define REG_SCPC_NID                            77                              //Регистр NID блока SCPC
//
#define REG_SCPC_EMULATION						78								//(1 - SCPC в режиме эмуляции, 0 - эмуляция SCPC выключена)
//
#define REG_PULSE_DURATION						80								// Задаваемая длительность импульса для SCPC (в мкс)
#define REG_PULSE_COUNT							81								// Количество импульсов
#define REG_PAUSE_DURATION						82								// Длительность паузы между импульсами (в мкс)
//
#define REG_SYNC_SWITCH							90								// Переключение вида выводимой синхронизации на передней панеле:
																				// 0 - синхронизация по вершине синуса
																				// 1 - синхро сигнал срабатывания ячеек

// Регистры только чтение
#define REG_DUT_U                               96                              //Измеренное значение прямого напряжения, мВ
#define REG_DUT_I_L                             97                              //Измеренное значение ударного тока, А (младшая часть)
#define REG_DUT_I_H                             98                              //Измеренное значение ударного тока, А (старшая часть)
#define REG_DEV_STATE                           99                              //Регистр состояния установки SCTU
#define REG_FAULT_REASON                        100                             //Регистр Fault
#define REG_WARNING                             101                             //Регистр Warning
#define REG_TOTAL_SCPC                          102                             //Количество блоков SCPC в установке
//
#define REG_SC_MAX_L                            103                             //Максимальное возможное значение ударного тока при текущей конфигурации (младшая часть)
#define REG_SC_MAX_H                            104                             //Максимальное возможное значение ударного тока при текущей конфигурации (старшая часть)
#define REG_INFO_SCPC_NID                       105                             //Nid, интересующего блока
#define REG_INFO_SCPC_PULSE_VALUE               106                             //Значение ударного тока, интересующего блока
#define REG_INFO_SCPC_BAT_VOLTAGE               107                             //Напряжение батареи, интересующего блока
#define REG_INFO_SCPC_DEV_STATE                 108                             //Состояние, интересующего блока
#define REG_INFO_SCPC_DEV_FAULT                 109                             //Значение Fault, интересующего блока
#define REG_INFO_SCPC_DEV_DISABLE               110                             //Значение Disable, интересующего блока
#define REG_INFO_SCPC_DEV_WARNING               111                             //Значение Warning, интересующего блока
#define REG_INFO_SCPC_DEV_PROBLEM               112                             //Значение Problem, интересующего блока
#define REG_INFO_K_SHUNT_AMP                    113                             //Текущее значение коэффициента усиления SCShuntAmplifier
//

#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record

// ENDPOINTS
//
#define EP16_Data_U		                1	                        // Оцифрованные данные ударного тока
#define EP16_Data_I		                2	                        // Оцифрованные данные прямого напряжения
#define EP16_RawData					3							// Сырые оцифрованные данные

//Fault
//
#define ERR_SCPC_NOT_ANS                        1                               //Блок SCPC не отвечает
#define ERR_SECURITY                            2                               //Открыта шторка безопасности
#define ERR_DUT_IS_NOT_OPEN                     3                               //Прибор не открылся
#define ERR_SCPC_PULSE                          4                               //Один из блоков не сформировал импульс ударного тока
#define ERR_SCPC_INTERNAL                       5                               //Внутренняя ошибка блока SCPC
#define ERR_WAVEFORM_TYPE                       6                               //Неправильно задан тип формы импульса ударного тока
#define ERR_SCPC_NOT_CHARGED					7								//блок SCPC не заряжен
//

//Warning
//
#define WARNING_I_OUT_OF_RANGE                  1                               //Измеренное значение тока вне диапазона
#define WARNING_U_OUT_OF_RANGE                  2                               //Измеренное значение напряжения вне диапазона
#define WARNING_SCPC_NOT_FIND                   3                               //Один или несколько блоков не определились при запуске
#define WARNING_SC_CUT_OFF                      4                               //Заданная амплитуда превышена и урезана до максимально возможного значения
#define WARNING_NID_NOT_FOUND                   5                               //Заданный Nid не найден в списке
#define WARNING_WRONG_VERSION                   6                               //Заданный блок версии 1.1 и не способен для формирования трапецеидального импульса
//

//User Errors
//
#define ERR_NONE                                0 				// Ошибок нет
#define ERR_CONFIGURATION_LOCKED                1 				// Устройство защищено от записи
#define ERR_OPERATION_BLOCKED                   2 				// Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY                    3 				// Устройство не готово для смены состояния
#define ERR_WRONG_PWD                           4 				// Неправильный ключ
//

// Password to unlock non-volatile area for write
//
#define ENABLE_LOCKING				FALSE
//

//Состояния блока SCH
typedef enum __DeviceState
{
  DS_None                                       = 0,                            //Блок в неопределенном состоянии
  DS_Fault                                      = 1,                            //Блок в состоянии Fault
  DS_Disabled                                   = 2,                            //Блок в состоянии Disabled
  DS_WaitTimeOut                                = 3,                            //Блок в ожидании таймаута между импульсами ударного тока
  DS_BatteryChargeWait                          = 4,                            //Блок в состоянии ожидания заряда конденсаторной батареи
  DS_Ready                                      = 5,                            //Блок в состоянии готовности
  DS_PulseConfigReady                           = 6,                            //Блок в сконфигурированном состоянии
  DS_PulseStart                                 = 7,                            //Блок в состоянии формирования импульса ударного тока
  DS_PulseEnd                                   = 8,                            //Блок завершил формирование импульса тока
  DS_PulseConfig                                = 9,                            //Блок в состоянии конфигурации
  DS_BatteryChargeStart                         = 10                            //Старт для заряда конденсатора
} DeviceState;
//

#endif // __DEV_OBJ_DIC_H
