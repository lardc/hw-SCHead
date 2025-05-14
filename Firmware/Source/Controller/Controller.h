#ifndef __CONTROLLER_H
#define __CONTROLLER_H

// Include
#include "stdinc.h"

// Types
typedef enum __DeviceState
{
	DS_None 			= 0,
	DS_Fault 			= 1,
	DS_Disabled 		= 2,
	DS_Ready 			= 3,
	DS_BatteryCharge 	= 4
} DeviceState;

typedef enum __DeviceSubState
{
	SS_None 			= 0,
	SS_WaitingSync 		= 1,
	SS_StartPulse		= 2
} DeviceSubState;

// Variables
extern volatile Int64U CONTROL_TimeCounter;
extern Int64U CONTROL_LEDTimeout;
extern Int64U CONTROL_RechargeTimeout;
extern Int64U CONTROL_AfterPulseTimeout;
extern Int64U CONTROL_SynchronizationTimeout;
extern Int64U CONTROL_PsBoardDisableTimeout;

extern Int16U CONTROL_ExtInfoCounter;
extern Int16U CONTROL_ExtInfoData[];

// Functions
void CONTROL_Init();
void CONTROL_Idle();
void CONTROL_CurrentEmergencyStop(Int16U Reason);
DeviceSubState CONTROL_GetSubState();
void CONTROL_SetDeviceState(DeviceState NewState);
void CONTROL_SetDeviceSubState(DeviceSubState NewSubState);
void CONTROL_InitBatteryChargeProcess();
void CONTROL_HandleFanLogic(bool IsImpulse);
void CONTROL_HandleLEDLogic(bool IsImpulse);
void CONTROL_FinishProcess();
void CONTROL_SwitchToProblem(Int16U Reason);

#endif // __CONTROLLER_H
