// -----------------------------------------
// Global definitions
// ----------------------------------------

#ifndef __GLOBAL_H
#define __GLOBAL_H

// Include
#include "stdinc.h"
#include "SysConfig.h"
#include "Constraints.h"

//Def
//------------------------------------------------
#define ADDRESS_FLAG_REGISTER                   0x20007FF0                      //Адрес регистра флагов
#define ADDRESS_LOADED_PROGRAMM_START           0x08004800                      //Адрес начала памяти загруженной программы
#define ADDRESS_FLASH_FIRST_PAGE                0x08000000                      //Адрес начала первой страницы FLASH памяти процессора
#define ADDRESS_FLASH_START_MCU					0x08004000						//Начальный адрес сектора FLASH памяти MCU для пользовательских задач
//------------------------------------------------
#define FLAG_RESET_FOR_PROG                     0x1111                          //Значение флага при перезагрузке процессора с последующим перепрограммированием
#define FLAG_LOAD_MAIN							0x3333							//Значение флага при нормальной загрузке прошивки
#define FLAG_RESET                              0x4444                          //Значение флага при перезагрузке процессора без перепрограммирования
//------------------------------------------------
#define MCU_FLASH_SIZE                          0x7FF                           //Размер FLASH памяти MCU для пользовательских задач
#define FLASH_PAGE_SIZE							0x800							//Размер страницы flash для записи DT
//------------------------------------------------
#define EP_WRITE_COUNT		                	1
#define EP_COUNT			        			3

#define SAMPLING_TAIL							100
#define VALUES_x_SIZE							(2500 + SAMPLING_TAIL)
#define ADC_BUFF_LENGTH							(VALUES_x_SIZE * 2)
#define SCALE_CURRENT_VALUE						50000							// Порог тока для масштабирования данных
#define EXTREMUM_POINTS							5								// Число точек усреднения экстремума
//------------------------------------------------

//Значения калибровочных коэффициентов по умолчанию
#define K_DUT_U_DEFAULT                         978                            //Калибровочный коэффициент по умолчанию по напряжению
#define K_DUT_I_DEFAULT                         712                             //Калибровочный коэффициент по умолчанию по току
#define U1_OFFSET_DEFAULT                       40                              //Смещение нуля канала 1 по напряжению
#define U2_OFFSET_DEFAULT                       40                              //Смещение нуля канала 1 по напряжению
#define K_SC_SET_DEFAULT                        1000                            //Множитель по умолчанию калибровочного уравнения значения задания тока
#define B_SC_SET_DEFAULT                        0                               //Смещение по умолчанию калибровочного уравнения значения задания тока
#define K_U_CAL_DEFAULT                         990                             //Множитель по умолчанию калибровочного уравнения измерения напряжения
#define B_U_CAL_DEFAULT                         42                              //Смещение по умолчанию калибровочного уравнения измерения напряжения
#define CHANNEL_DEFAULT                         2                               //Канал измерения по умолчанию
#define I1_OFFSET_DEFAULT                       74                              //Смещение нуля канала 1 по току
#define I2_OFFSET_DEFAULT                       72                              //Смещение нуля канала 1 по току
#define R_SHUNT_DEFAULT                         25                              //Сопротивление шунта по умолчанию 100 мкОм
//
#define UTM_I_MAX                               15500                           //Максимальное значение тока при котором вводится поправка к заданию
#define SCTU_SC_VALUE_MIN                       100                             //Минимальное значение ударного тока SCTU, A
#define SCTU_SC_SINE_MAX  		                120000                          //Максимальное значение ударного тока установки SCTU при полусинусоидальной форме импульса, A
#define SCTU_SC_TRAPEZE_MAX                     16000                           //Максимальное значение ударного тока установки SCTU при трапецеидальной форме импульса, A
#define TRAPEZE_EDGE_TIME_MIN                   100                             //Минимальное фремя фронта трапеции, мкС
#define TRAPEZE_EDGE_TIME_MAX                   1000                            //Максимальное фремя фронта трапеции, мкС
#define ADC_REF_MV                              2980                            //Опорное напряжение АЦП, мВ
#define WAVEFORM_SINE                           0xAAAA                          //Полусинусоидальная форма ударного тока
#define WAVEFORM_TRAPEZE                        0xBBBB                          //Трапециедальная форма ударного тока
#define CHANNEL_1                               1                               //Канал измерения 1
#define CHANNEL_2                               2                               //Канал измерения 2
#define DIODE                                   1234                            //Тип прибора - диод
#define THYRISTOR                               5678                            //Тип прибора - тиристор
#define TIME_CHANGE_STATE                       1000                            //Через 5 сек после импульса можно менять статус блока
#define GLOBAL_TIMEOUT							60000							//Время такта работы установки
#define WAIT_TIMEOUT_VALUE						(GLOBAL_TIMEOUT-TIME_CHANGE_STATE)
#define VOLTAGE_MEASURE_MAX                     4000                            //Максимальное измеряемое напряжение, мВ
#define SCTU_NUM_MAX                            39                              //Максимально возможно число блоков SCPC в установке SCTU
#define ADC_I_SAMPLE_THRESHOLD                  5                               //Порог количества сэмплов с одинаоковой амплитудой для определения амплитуды тока
#define AVERAGE_POINTS                          100                             //Количество точек усреднения
#define SCPC_PREPULSE_PAUSE						9		// Фиксированная задержка в SCPC перед формированием импульса, мс

//Переменные
extern volatile Int64U CONTROL_TimeCounter;
extern Int64U LED_BlinkTimeCounter;
extern volatile uint16_t ADC_BUF[ADC_BUFF_LENGTH];

#endif // __GLOBAL_H
