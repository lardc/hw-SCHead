#ifndef __BOARD_H
#define __BOARD_H

#include "stm32f30x.h"

#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwNCAN.h"
#include "ZwTIM.h"
#include "ZwDAC.h"
#include "ZwDMA.h"
#include "ZwADC.h"
#include "ZwEXTI.h"
#include "ZwSCI.h"
#include "ZwIWDG.h"
#include "ZwNFLASH.h"

// Определения для выходных портов
GPIO_PortPinSettingMacro GPIO_RCK			= {GPIOA, Pin_4};
GPIO_PortPinSettingMacro GPIO_SRCK			= {GPIOA, Pin_5};
GPIO_PortPinSettingMacro GPIO_DATA			= {GPIOA, Pin_7};
GPIO_PortPinSettingMacro GPIO_FAN			= {GPIOB, Pin_0};
GPIO_PortPinSettingMacro GPIO_LED			= {GPIOB, Pin_4};
GPIO_PortPinSettingMacro GPIO_LED_EXT		= {GPIOB, Pin_1};
GPIO_PortPinSettingMacro GPIO_BAT_CHARGE	= {GPIOB, Pin_2};
GPIO_PortPinSettingMacro GPIO_HVPS_CTRL		= {GPIOB, Pin_10};
GPIO_PortPinSettingMacro GPIO_MW_CTRL		= {GPIOB, Pin_11};

// Определения для портов альтернативных функций
GPIO_PortPinSettingMacro GPIO_ALT_CAN_RX	= {GPIOA, Pin_11};
GPIO_PortPinSettingMacro GPIO_ALT_CAN_TX	= {GPIOA, Pin_12};
GPIO_PortPinSettingMacro GPIO_ALT_UART_RX	= {GPIOA, Pin_10};
GPIO_PortPinSettingMacro GPIO_ALT_UART_TX	= {GPIOA, Pin_9};

// Линия синхронизации
GPIO_PortPinSettingMacro GPIO_SYNC			= {GPIOA, Pin_8};
GPIO_PortPinSettingMacro GPIO_VOUT_STATE	= {GPIOB, Pin_15};

#endif // __BOARD_H
