// Header
#include "LowLevel.h"
// Include
#include "Board.h"
#include "Delay.h"
#include "DataTable.h"

// Functions
//
void LL_ToggleBoardLED()
{
	GPIO_Toggle(GPIO_LED);
}
//-----------------------------

void LL_Fan(bool State)
{
	GPIO_SetState(GPIO_FAN, State);
}
//-----------------------------

void LL_BatteryDischarge(bool State)
{
	// Разряд происходит при низком уровне на пине
	GPIO_SetState(GPIO_BAT_CHARGE, !State);
}
//-----------------------------

void LL_ExternalLED(bool State)
{
	GPIO_SetState(GPIO_LED_EXT, State);
}
//-----------------------------

void LL_ToggleExternalLED()
{
	GPIO_Toggle(GPIO_LED_EXT);
}
//-----------------------------

void LL_MeanWellRelay(bool State)
{
	GPIO_SetState(GPIO_MW_CTRL, State);
}
//-----------------------------

void LL_PSBoardOutput(bool State)
{
	// Включение питания происходит при низком уровне на пине
	GPIO_SetState(GPIO_HVPS_CTRL, !State);
}
//-----------------------------

void LL_SoftSpiSRCK(bool State)
{
	GPIO_SetState(GPIO_SRCK, State);
}
//-----------------------------

void LL_SoftSpiRCK(bool State)
{
	GPIO_SetState(GPIO_RCK, State);
}
//-----------------------------

void LL_SoftSpiData(bool State)
{
	GPIO_SetState(GPIO_DATA, State);
}
//-----------------------------

void LL_WriteToGateRegister(uint16_t Data)
{
	LL_WriteWordToGateRegister(Data);
	LL_FlipSpiRCK();
	LL_WriteWordToGateRegister(0);
}
//-----------------------------

void LL_FlipSpiRCK()
{
	LL_SoftSpiRCK(TRUE);
	DELAY_US(1);
	LL_SoftSpiRCK(FALSE);
}
//-----------------------------

void LL_WriteWordToGateRegister(uint16_t Word)
{
	for(int i = 15; i >= 0; i--)
	{
		LL_SoftSpiData((Word >> i) & 0x1);
		DELAY_US(1);
		LL_SoftSpiSRCK(TRUE);
		DELAY_US(1);
		LL_SoftSpiSRCK(FALSE);
	}
}
//-----------------------------

void LL_ForceSYNC(bool State)
{
	// Синхронизация происходит при низком уровне на пине
	GPIO_SetState(GPIO_SYNC, !State);
}
//-----------------------------

bool LL_GetSYNCState()
{
	return !GPIO_GetState(GPIO_SYNC);
}
//-----------------------------

bool LL_IsOutputVoltageHigh()
{
	return DataTable[REG_USE_OUT_VOLTAGE_MONITOR] ? GPIO_GetState(GPIO_VOUT_STATE) : FALSE;
}
//-----------------------------

void LL_PulseSYNC()
{
	LL_ForceSYNC(true);
	DELAY_US(90);
	LL_ForceSYNC(false);
}
//-----------------------------
