#include "InitConfig.h"
#include "Board.h"
#include "SysConfig.h"
#include "BCCIxParams.h"

// Definition
//
#define EXTI8_INTERRUPT_PRIORITY	27
#define EXTI15_INTERRUPT_PRIORITY	30
#define TIM3_INTERRUPT_PRIORITY		36
#define USART1_INTERRUPT_PRIORITY	44
#define CAN_INTERRUPT_PRIORITY		47

// Functions
//
Boolean SysClk_Config()
{
	return RCC_PLL_HSE_Config(QUARTZ_FREQUENCY, PREDIV_4, PLL_14);
}
//------------------------------------------------

void EI_Config()
{
	EXTI_Config(EXTI_PA, EXTI_8, BOTH_TRIG, 0);
	EXTI_Config(EXTI_PB, EXTI_15, BOTH_TRIG, 0);
	//
	EXTI_EnableInterrupt(EXTI9_5_IRQn, 0, true);
	EXTI_EnableInterrupt(EXTI15_10_IRQn, 0, true);

	NVIC_SetPriority(EXTI9_5_IRQn, EXTI8_INTERRUPT_PRIORITY);
	NVIC_SetPriority(EXTI15_10_IRQn, EXTI15_INTERRUPT_PRIORITY);
}
//------------------------------------------------

void IO_Config()
{
	// Включение тактирования портов
	RCC_GPIO_Clk_EN(PORTA);
	RCC_GPIO_Clk_EN(PORTB);
	
	// Аналаговые входы
	GPIO_Config(GPIOA, Pin_3, Analog, NoPull, HighSpeed, NoPull);
	
	// Цифровые входы
	GPIO_InitInput(GPIO_VOUT_STATE, NoPull);

	
	// Выходы
	GPIO_InitPushPullOutput(GPIO_RCK);
	GPIO_InitPushPullOutput(GPIO_SRCK);
	GPIO_InitPushPullOutput(GPIO_DATA);
	GPIO_InitPushPullOutput(GPIO_FAN);
	GPIO_InitPushPullOutput(GPIO_LED);
	GPIO_InitPushPullOutput(GPIO_LED_EXT);
	GPIO_InitPushPullOutput(GPIO_BAT_CHARGE);
	GPIO_InitPushPullOutput(GPIO_HVPS_CTRL);
	GPIO_InitPushPullOutput(GPIO_MW_CTRL);
	
	//Выходы с ОК
	GPIO_InitOpenDrainOutput(GPIO_SYNC, NoPull);
	GPIO_SetState(GPIO_SYNC, true);

	// Альтернативные функции
	GPIO_InitAltFunction(GPIO_ALT_CAN_RX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_CAN_TX, AltFn_9);
	GPIO_InitAltFunction(GPIO_ALT_UART_RX, AltFn_7);
	GPIO_InitAltFunction(GPIO_ALT_UART_TX, AltFn_7);
}
//------------------------------------------------

void UART_Config()
{
	USART_Init(USART1, SYSCLK, USART_BAUDRATE);
	USART_Recieve_Interupt(USART1, 0, true);
	NVIC_SetPriority(EXTI15_10_IRQn, USART1_INTERRUPT_PRIORITY);
}
//------------------------------------------------

void ADC_Init()
{
	RCC_ADC_Clk_EN(ADC_12_ClkEN);
	
	ADC_Calibration(ADC1);
	ADC_SoftTrigConfig(ADC1);
	ADC_Enable(ADC1);
}
//------------------------------------------------

void Timer7_Config()
{
	TIM_Clock_En(TIM_7);
	TIM_Config(TIM7, SYSCLK, TIMER7_uS);
	TIM_Interupt(TIM7, 0, true);
	TIM_Start(TIM7);
}
//------------------------------------------------

void Timer3_Config()
{
	TIM_Clock_En(TIM_3);
	TIM_Config(TIM3, SYSCLK, TIMER3_uS);
	TIM_Interupt(TIM3, 0, true);
	NVIC_SetPriority(EXTI15_10_IRQn, TIM3_INTERRUPT_PRIORITY);
	TIM_Stop(TIM3);
}
//------------------------------------------------

void WatchDog_Config()
{
	IWDG_Config();
	IWDG_ConfigureSlowUpdate();
}
//------------------------------------------------

void INITCFG_ConfigCAN(Int16U NodeID)
{
	Int32U Mask = ((Int32U)NodeID) << CAN_SLAVE_NID_MPY;

	RCC_CAN_Clk_EN(CAN_1_ClkEN);
	NCAN_Init(SYSCLK, CAN_BAUDRATE, FALSE);
	NCAN_FIFOInterrupt(TRUE);
	NCAN_FilterInit(0, Mask, Mask);
	NCAN_InterruptSetPriority(0);
}
//------------------------------------------------
