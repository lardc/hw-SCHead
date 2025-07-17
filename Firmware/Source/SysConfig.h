#ifndef __SYSCONFIG_H
#define __SYSCONFIG_H
// -----------------------------------------
// System parameters

// Include
#include <ZwBase.h>
#include "ZwGPIO.h"

// Flash loader options
#define BOOT_LOADER_VARIABLE			(*((volatile uint32_t *)0x20000000))
#define BOOT_LOADER_REQUEST				0x12345678
#define BOOT_LOADER_MAIN_PR_ADDR		0x08004800
//-----------------------
//Define
//
#define SYSCLK                                  70000000                        //Тактовая частота системной шины процессора
#define QUARTZ_FREQUENCY                        20000000                        //Частота кварца
//-----------------------
//USART
#define USART_BAUDRATE                          115200                          //Скорость USART
#define USART_FIFOlen                           32                              //Длина FIFO USART
//-----------------------
//Timer
#define TIMER15_uS                              50                              //Период работы таймера 15
#define TIMER3_uS                               1000                            //Период работы таймера 3
//-----------------------
//CAN
#define CAN_BAUDRATE                            100000                          //Битрейт CAN
#define MASTER_DEVICE_CAN_ADDRESS				8                               //NodeID в режиме master
#define SLAVE_DEVICE_CAN_ADDRESS				8                               //NodeID в режиме slave
#define TYPE_DEVICE                             MASTER_DEVICE                   //Тип устройства master/slave
//-----------------------
//Порты
#define SECURITY_SENSOR                         GPIO_Read_Bit(GPIOB, Pin_14)
#define CURRENT_DUT_CONTROL                     GPIO_Read_Bit(GPIOB, Pin_6)
#define LED                                     GPIO_Read_Bit(GPIOB, Pin_12)
#define LED_0                                   GPIO_Bit_Rst(GPIOB, Pin_12)
#define LED_1                                   GPIO_Bit_Set(GPIOB, Pin_12)
#define SCPC_SYNC_SIGNAL_START                  GPIO_Bit_Rst(GPIOA, Pin_8);
#define SCPC_SYNC_SIGNAL_STOP                   GPIO_Bit_Set(GPIOA, Pin_8);
#define OSC_SYNC_SIGNAL_START                   GPIO_Bit_Rst(GPIOB, Pin_15);
#define OSC_SYNC_SIGNAL_STOP                    GPIO_Bit_Set(GPIOB, Pin_15);
#define DUT_OPEN                                GPIO_Bit_Rst(GPIOB, Pin_13);
#define DUT_CLOSE                               GPIO_Bit_Set(GPIOB, Pin_13);
#define CH_1_SET                                GPIO_Bit_Rst(GPIOA, Pin_15)
#define CH_2_SET                                GPIO_Bit_Set(GPIOA, Pin_15)
#define SPI1_CS_1                               GPIO_Bit_Set(GPIOA, Pin_4);
#define SPI1_CS_0                               GPIO_Bit_Rst(GPIOA, Pin_4);
//-----------------------
// Параметры конфигурации CAN MailBox
#define ZW_CAN_RECEIVE_MAILBOX		        TRUE
#define ZW_CAN_TRANSMIT_MAILBOX		        FALSE
#define ZW_CAN_MSG_LENGTH	    	        8
#define ZW_CAN_NO_PRIORITY			0
#define ZW_CAN_STRONG_MATCH			0
//-----------------------
// Флаги конфигурации MailBox
#define ZW_CAN_RTRAnswer                        0x01
#define ZW_CAN_MBProtected                      0x02
#define ZW_CAN_UseExtendedID                    0x04
//-----------------------
// Ключ для разблокировки записи в энергонезависимую память
#define ENABLE_LOCKING				FALSE
#define	SCCI_TIMEOUT_TICKS  		1000
#define	BCCIM_TIMEOUT_TICKS			50				// Таймаут протоколоа мастер BCCI (в мс)
//

#endif // __SYSCONFIG_H
