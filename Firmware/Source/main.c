#include "main.h"
#include "BCCIxParams.h"

int main()
{
  SetVectorTable();

  //Настройка системной частоты тактирования
  SysClk_Config();

  //Настройка портов
  IO_Config();

  //Настройка UART
  UART_Config();

  //Настройка CAN
  CAN_Config();

  //Настройка SPI
  SPI_Config();

  //Timer 3
  Timer3_Config();

  //Timer 15
  Timer15_Config();

  //Настройка АЦП
  ADC_Init();

  //Настройка DMA
  DMA_Config();

  //Настройка сторожевого таймера
  WatchDog_Config();

  //Инициализация логики работы контроллера
  CONTROL_Init();

  //Основной цикл
  while(true)
	CONTROL_Idle();

  return 0;
}

//------------------------------------------------------------------------------
void WatchDog_Config(void)
{
  IWDG_Config();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void SysClk_Config(void)
{
	RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
	RCC_SysCfg_Clk_EN();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void IO_Config(void)
{
  //Включение тактирования портов
  RCC_GPIO_Clk_EN(PORTA);                                                       //GPIOA
  RCC_GPIO_Clk_EN(PORTB);                                                       //GPIOB
  //

  //Цифровые входы
  GPIO_Config(GPIOB,Pin_14,Input,NoPull,HighSpeed,NoPull);                      //PB14 - Security sensor
  GPIO_Config(GPIOB,Pin_6,Input,NoPull,HighSpeed,NoPull);                       //PB6 - Current DUT

  //Аналоговые входы
  GPIO_Config(GPIOA,Pin_2,Analog,NoPull,HighSpeed,NoPull);                      //PA2 - Udut
  GPIO_Config(GPIOA,Pin_3,Analog,NoPull,HighSpeed,NoPull);                      //PA3 - Idut


  //Выходы
  GPIO_Config(GPIOB,Pin_12,Output,PushPull,HighSpeed,NoPull);                   //PB12(LED)
  GPIO_Config(GPIOA,Pin_8,Output,OpenDrain,HighSpeed,Pull_Up);                  //PA8(SYNC_SCPC)
  GPIO_Bit_Set(GPIOA, Pin_8);
  GPIO_Config(GPIOB,Pin_15,Output,OpenDrain,HighSpeed,Pull_Up);                 //PB15(SYNC_OSC)
  GPIO_Bit_Set(GPIOB, Pin_15);
  GPIO_Config(GPIOB,Pin_13,Output,PushPull,HighSpeed,NoPull);                   //PB13(DUT_EN)
  GPIO_Bit_Set(GPIOB, Pin_13);
  GPIO_Config(GPIOA,Pin_15,Output,PushPull,HighSpeed,NoPull);                   //PA15(CH_SELECT)
  GPIO_Config(GPIOA,Pin_4,Output,PushPull,HighSpeed,NoPull);                    //PA4(SPI1_CS)


  //Альтернативные функции портов
  GPIO_Config(GPIOA,Pin_9,AltFn,PushPull,HighSpeed,NoPull);                     //PA9(USART1 TX)
  GPIO_AltFn(GPIOA, Pin_9, AltFn_7);                                            //Alternate function PA9 enable

  GPIO_Config(GPIOA,Pin_10,AltFn,PushPull,HighSpeed,NoPull);                    //PA10(USART1 RX)
  GPIO_AltFn(GPIOA, Pin_10, AltFn_7);                                           //Alternate function PA10 enable

  GPIO_Config(GPIOA,Pin_11,AltFn,PushPull,HighSpeed,NoPull);                    //PA11(CAN RX)
  GPIO_AltFn(GPIOA, Pin_11, AltFn_9);                                           //Alternate function PA11 enable

  GPIO_Config(GPIOA,Pin_12,AltFn,PushPull,HighSpeed,NoPull);                    //PA12(CAN TX)
  GPIO_AltFn(GPIOA, Pin_12, AltFn_9);                                           //Alternate function PA12 enable

  GPIO_Config(GPIOA,Pin_5,AltFn,PushPull,HighSpeed,NoPull);                     //PA5 - SPI_SCK
  GPIO_AltFn(GPIOA, Pin_5, AltFn_5);

  GPIO_Config(GPIOA,Pin_7,AltFn,PushPull,HighSpeed,NoPull);                     //PA7 - SPI_MOSI
  GPIO_AltFn(GPIOA, Pin_7, AltFn_5);
  //
}
//------------------------------------------------------------------------------

//----------------------------Настройка SPI-------------------------------------
void SPI_Config(void)
{
  SPIx_Config(SPI1);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void CAN_Config(void)
{
	 NCAN_Init(CAN_1_ClkEN,SYSCLK,CAN_BAUDRATE);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void UART_Config(void)
{
  USART_Init(USART1, SYSCLK, USART_BAUDRATE);
  USART_Recieve_Interupt(USART1, 1, true);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void ADC_Init(void)
{
  RCC_ADC_Clk_EN(ADC_12_ClkEN);
  ADC_Calibration(ADC1);
  ADC_TrigConfig(ADC1, ADC12_TIM15_TRGO, RISE);
  ADC_ChannelSeqLen(ADC1, 2);
  ADC_ChannelSet_Sequence1_4(ADC1, 3, 1);
  ADC_ChannelSet_Sequence1_4(ADC1, 4, 2);
  ADC_DMAConfig(ADC1);
  ADC_Enable(ADC1);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Timer15_Config(void)
{
  TIM_Clock_En(TIM_15);
  TIM_Config(TIM15, SYSCLK, TIMER15_uS);
  TIM_MasterMode(TIM15, MMS_UPDATE);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void DMA_Config(void)
{
  DMA_Clk_Enable(DMA_ClkEN);
  DMA_Reset(DMA1_Channel1);
  DMA_Interrupt(DMA1_Channel1, DMA_TRANSFER_COMPLETE, 0, true);
  DMA1ChannelX_DataConfig(DMA1_Channel1, (uint32_t)(&ADC_BUF[0]), (uint32_t)(&ADC1->DR), ADC_BUFF_LENGTH);
  DMA1ChannelX_Config(DMA1_Channel1,DMA_MEM2MEM_DIS,DMA_LvlPriority_LOW,DMA_MSIZE_16BIT,DMA_PSIZE_16BIT,
                      DMA_MINC_EN,false,DMA_CIRCMODE_EN,DMA_READ_FROM_PERIPH,DMA_CHANNEL_EN);
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void SetVectorTable()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Timer3_Config(void)
{
  TIM_Clock_En(TIM_3);
  TIM_Config(TIM3, SYSCLK, TIMER3_uS);
  TIM_Interupt(TIM3, 1, true);
  TIM_Start(TIM3);
}
//------------------------------------------------------------------------------


