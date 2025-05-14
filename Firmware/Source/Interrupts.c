// Include
#include "Interrupts.h"
//
#include "Controller.h"
#include "LowLevel.h"
#include "Board.h"
#include "SysConfig.h"
#include "Global.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
//

// Variables
//
bool Impulse = false;

// Functions prototypes
void INT_SyncTimeoutControl(bool State);
//

// Functions
//
void EXTI9_5_IRQHandler()
{
	switch (CONTROL_GetSubState())
	{
		case SS_WaitingSync:
			Impulse = true;

			CONTROL_SetDeviceSubState(SS_StartPulse);
			INT_SyncTimeoutControl(true);

			CONTROL_HandleFanLogic(Impulse);
			CONTROL_HandleLEDLogic(Impulse);
			break;

		case SS_StartPulse:
			Impulse = false;

			INT_SyncTimeoutControl(false);
			CONTROL_FinishProcess();
			break;

		case SS_None:
			break;
	}

	EXTI_FlagReset(EXTI_8);
}
//-----------------------------------------

void EXTI15_10_IRQHandler()
{
	if(CONTROL_GetSubState() == SS_StartPulse)
		INT_SyncTimeoutControl(!LL_IsOutputVoltageHigh());

	EXTI_FlagReset(EXTI_15);
}
//-----------------------------------------

void TIM3_IRQHandler()
{
	if(TIM_StatusCheck(TIM3))
	{
		INT_SyncTimeoutControl(false);

		if(Impulse && !LL_IsOutputVoltageHigh())
			CONTROL_SwitchToProblem(PROBLEM_SYNC_TOO_LONG);

		TIM_StatusClear(TIM3);
	}
}
//-----------------------------------------

void USART1_IRQHandler()
{
	if(ZwSCI_RecieveCheck(USART1))
	{
		ZwSCI_RegisterToFIFO(USART1);
		ZwSCI_RecieveFlagClear(USART1);
	}
}
//-----------------------------------------
void USB_LP_CAN_RX0_IRQHandler()
{
	if(NCAN_RecieveCheck())
	{
		NCAN_RecieveData();
		NCAN_RecieveFlagReset();
	}
}
//-----------------------------------------

void TIM7_IRQHandler()
{
	static uint16_t LED_BlinkTimeCounter = 0;

	if(TIM_StatusCheck(TIM7))
	{
		CONTROL_TimeCounter++;
		if(++LED_BlinkTimeCounter > TIME_LED_BLINK)
		{
			LL_ToggleBoardLED();
			LED_BlinkTimeCounter = 0;
		}

		CONTROL_HandleFanLogic(Impulse);

		TIM_StatusClear(TIM7);
	}
}
//-----------------------------------------

void INT_SyncTimeoutControl(bool State)
{
	State ? TIM_Start(TIM3) : TIM_Stop(TIM3);
	TIM_Reset(TIM3);
}
//-----------------------------------------
