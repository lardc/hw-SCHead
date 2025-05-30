// -----------------------------------------
// Logic controller
// ----------------------------------------

#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "stdinc.h"
#include "Global.h"
#include "ZwCAN.h"
#include "BCCIMaster.h"
#include "ZwIWDG.h"
#include "ZwADC.h"
#include "ZwDMA.h"
#include "ZwSPI.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "ShuntAmplifier.h"
#include "SCPC.h"
#include "BCCIxParams.h"
//

//Defines
#define DELAY_PULSE_START			9 + DataTable[REG_PULSE_DURATION]
//


// Переменные
//
extern Int16U CONTROL_Values_U[VALUES_x_SIZE];
extern Int16U CONTROL_Values_I[VALUES_x_SIZE];
extern Int16U CONTROL_Values_U_Counter;
extern Int16U CONTROL_Values_I_Counter;
//


// Функции
//
void CONTROL_Init();
void CONTROL_Idle();
void SetDeviceState(DeviceState NewState);
void SetDeviceFault(Int16U Fault);
void Delay_mS(uint64_t Delay);
void IWDG_Control(void);
void SCTU_Config(pBCCIM_Interface Interface);
void SurgeCurrentProcess(pBCCIM_Interface Interface);
void ErrorsClear(pBCCIM_Interface Interface);
uint16_t ReadMailBox(uint16_t Nid, uint16_t MBox, bool ErrorCtrl);
void UI_Dut_MeasureStart(void);
void Utm_Measure();
void SCTU_PulseSineConfig(pBCCIM_Interface Interface);
void SCTU_PulseTrapezeConfig(pBCCIM_Interface Interface);
void MeasureChannelSet(uint16_t ChannelNumber, bool ChannelInverse);
//

#endif // __CONTROLLER_H
