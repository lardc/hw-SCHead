#ifndef __DEV_OBJ_DIC_H
#define __DEV_OBJ_DIC_H

// Команды
#define ACT_ENABLE_POWER				1	// Переход в состояние ожидания
#define ACT_DISABLE_POWER				2	// Отключение блока
#define ACT_FAULT_CLEAR					3	// Очистка fault
#define ACT_WARNING_CLEAR				4	// Очистка warning

#define ACT_DBG_FAN						10	// Импульсное включение вентилятора
#define ACT_DBG_DISCHARGE				11	// Импульсное включение реле разряда конденстаторов
#define ACT_DBG_EXT_LED					12	// Импульсное включение внешней индикации
#define ACT_DBG_MW_RELAY				13	// Импульсное включение реле питания БП MeanWell
#define ACT_DBG_PSBOARD_OUTPUT			14	// Импульсное включение сигнала для PSBOARD
#define ACT_DBG_GATE_CONTROL			15	// Запись значения в сдвиговый регистр управления затворами
#define ACT_DBG_GATE_EN					16	// Включение силовых транзисторов

#define ACT_VOLTAGE_CONFIG				100	// Конфигурация напряжения и настройка затворов транзисторов
#define ACT_PULSE_CONFIG				101	// Выключить PsBoard перед подачей импульса
#define ACT_SW_PULSE					102	// Программный запуск импульса тока

#define ACT_SAVE_TO_ROM					200	// Сохранение пользовательских данных во FLASH процессора
#define ACT_RESTORE_FROM_ROM			201	// Восстановление данных из FLASH
#define ACT_RESET_TO_DEFAULT			202	// Сброс DataTable в состояние по умолчанию

#define ACT_BOOT_LOADER_REQUEST			320	// Перезапуск процессора с целью перепрограммирования

#define ACT_JSON_INIT_READ				341	// Инициализация начала считывания JSON
#define ACT_JSON_TO_EP					342	// Выполнить чтение шаблона JSON в EP
// -----------------------------

// Регистры
// Сохраняемые регистры
// 0 -1
#define REG_VOLTAGE_HYST				2	// Гистерезис регулирования напряжения (в % / 10)
#define REG_BAT_CHARGE_TIMEOUT			3	// Таймаут выхода батареи на заданное напряжение (в мс)
#define REG_FAN_OPERATE_TIME			4	// Время работы вентилятора после импульса (в с)
#define REG_FAN_OPERATE_PERIOD			5	// В простое вентилятор включается не реже чем (в с)
#define REG_PS_BOARD_DISABLE_TIMEOUT	6	// Таймаут дозаряда после импульса, мс
#define REG_RESISTANCE_PER_LSB			7	// Сопротивление наименьшего значащего разряда (в Ом * 100)
#define REG_MAX_CURRENT_PER_BIT			8	// Максимальный ток на 1 бит (в А)
#define REG_SYNC_WAIT_TIMEOUT			9	// Таймаут ожидания импульса синхронизации, мс
#define REG_AFTER_PULSE_TIMEOUT			10	// Таймаут после импульса, мс
#define REG_GATE_RESOLUTION				11	// Разрешение регистра драйверов затворов
//
#define REG_V_BAT_OFFSET				20	// Смещение оцифрованного напряжения батареи 1 (в тиках)
#define REG_V_BAT_K						21	// Коэффициент пересчёта напряжения АЦП (в мВ) в напряжение батареи (в В) x1000
#define REG_V_BAT_P2					22	// Калибровочный коэффициент P2 измерения напряжения батареи
#define REG_V_BAT_P1					23	// Калибровочный коэффициент P1 измерения напряжения батареи
#define REG_V_BAT_P0					24	// Калибровочный коэффициент P0 измерения напряжения батареи
// 25 - 29
#define REG_USE_OUT_VOLTAGE_MONITOR		32	// Включение мониторинга напряжения
// 31 - 59
#define REG_CFG_NODE_ID					60	// Настройка CAN NodeID

// Несохраняемы регистры чтения-записи
#define REG_VOLTAGE_SETPOINT			128	// Значение задания напряжения (в В)
#define REG_GATE_REGISTER				129	// Значение-маска конфигурации затворов
//
#define REG_DBG							160	// Отладочный регистр

// Регистры только чтение
#define REG_DEV_STATE					192	// Регистр состояния
#define REG_FAULT_REASON				193	// Регистр Fault
#define REG_DISABLE_REASON				194	// Регистр Disable
#define REG_WARNING						195	// Регистр Warning
#define REG_PROBLEM						196	// Регистр Problem
#define REG_OP_RESULT					197	// Регистр результата операции
#define REG_DEV_SUBSTATE				198
//
#define REG_ACTUAL_BAT_VOLTAGE			200	// Текущее напряжение на батарее (в В)
// -----------------------------
#define REG_FWINFO_SLAVE_NID			256	// Device CAN slave node ID
#define REG_FWINFO_MASTER_NID			257	// Device CAN master node ID (if presented)
// 258 - 259
#define REG_FWINFO_STR_LEN				260	// Length of the information string record
#define REG_FWINFO_STR_BEGIN			261	// Begining of the information string record

// Operation results
#define OPRESULT_NONE					0	// No information or not finished
#define OPRESULT_OK						1	// Operation was successful
#define OPRESULT_FAIL					2	// Operation failed

//  Fault and disable codes
#define DF_NONE							0
#define DF_BATTERY						1	// Проблема заряда батареи

// Problem
#define PROBLEM_NONE					0
#define PROBLEM_SYNC_TOO_LONG			1	// Превышена длительность импульса синхронизации
#define PROBLEM_GATE_REGISTER			2	// Некорректный код регистра затворов
#define PROBLEM_SYNC_LINE				3	// Некорректное состояние линии синхронизации

//  Warning
#define WARNING_NONE					0

//  User Errors
#define ERR_NONE						0
#define ERR_CONFIGURATION_LOCKED		1	//  Устройство защищено от записи
#define ERR_OPERATION_BLOCKED			2	//  Операция не может быть выполнена в текущем состоянии устройства
#define ERR_DEVICE_NOT_READY			3	//  Устройство не готово для смены состояния
#define ERR_WRONG_PWD					4	//  Неправильный ключ

// EP
#define EP16_ExtInfoData				20	// Diag data drom flash

#endif //  __DEV_OBJ_DIC_H
