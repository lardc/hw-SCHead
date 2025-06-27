//Include
#include "stm32f30x.h"
#include "stdbool.h"
#include "LocalDefs.h"
#include "Global.h"
#include "ZwTIM.h"
#include "DataTable.h"
#include "ZwGPIO.h"
#include "BCCIMaster.h"
#include "ZwNCAN.h"
#include "Controller.h"
#include "DeviceProfile.h"
#include "ZwDMA.h"
#include "ZwSCI.h"
//

//Define
#define LED_BLINK_TIME          250             //Период моргания светодиода на плате
//

//-----------------------------------------
void USART1_IRQHandler()
{
  if(ZwSCI_RecieveCheck(USART1))
  	ZwSCI_RegisterToFIFO(USART1);

  ZwSCI_RecieveFlagClear(USART1);
}
//-----------------------------------------

//-----------------------------------------
void USB_LP_CAN_RX0_IRQHandler()
{
	if (NCAN_RecieveCheck())
	{
		NCAN_RecieveData();
		NCAN_RecieveFlagReset();
	}
}
//-----------------------------------------

//-----------------------------------------
void TIM3_IRQHandler(void)
{
  if (TIM_StatusCheck(TIM3))
  {
    CONTROL_TimeCounter++;

    if(CONTROL_TimeCounter>(LED_BlinkTimeCounter+LED_BLINK_TIME))
    {
      //Моргаем светодиодом
      if(LED==1)LED_0;
      else LED_1;

      LED_BlinkTimeCounter = CONTROL_TimeCounter;
    }
    TIM_StatusClear(TIM3);
  }
}
//-----------------------------------------


//-----------------------------------------
void DMA1_Channel1_IRQHandler(void)
{
    TIM_Stop(TIM15);
    DMA_TransferCompleteFlagReset(DMA_IFCR_CTCIF1);
}
//-----------------------------------------
