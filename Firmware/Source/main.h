#ifndef __MAIN_H
#define __MAIN_H

//Includes
#include "stm32f30x.h"
#include "ZwRCC.h"
#include "ZwGPIO.h"
#include "ZwCAN.h"
#include "Global.h"
#include "ZwSCI.h"
#include "ZwTIM.h"
#include "ZwIWDG.h"
#include "Controller.h"
#include "ZwDMA.h"
#include "ZwSPI.h"


//Functions
extern void CONTROL_Idle();
Boolean SysClk_Config(void);
void IO_Config(void);
void CAN_Config(void);
void UART_Config();
void InitializeController(Boolean GoodClock);
void CONTROL_Init(void);
void Timer6_Config(void);
void Timer7_Config(void);
void WatchDog_Config(void);
void SetVectorTable(uint32_t Address);
void Timer3_Config(void);
void ADC_Init(void);
void DMA_Config(void);
void Timer15_Config(void);
void SPI_Config(void);
#endif //__MAIN_H
