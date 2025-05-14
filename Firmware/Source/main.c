#include "Controller.h"
#include "InitConfig.h"
#include "SysConfig.h"

int main()
{
	__disable_irq();
	SCB->VTOR = (uint32_t)BOOT_LOADER_MAIN_PR_ADDR;
	__enable_irq();
	
	// Настройка системной частоты тактирования
	SysClk_Config();
	
	// Настройка портов
	IO_Config();
	
	// Настройка внешних прерываний
	EI_Config();
	
	// Настройка UART
	UART_Config();
	
	// Настройка системного счетчика
	Timer7_Config();
	
	// Настройка таймера таймаута длительности сихронизации
	Timer3_Config();

	// Настройка АЦП
	ADC_Init();
	
	// Настройка сторожевого таймера
	WatchDog_Config();
	
	// Инициализация логики контроллера
	CONTROL_Init();
	
	// Фоновый цикл
	while(TRUE)
		CONTROL_Idle();
	
	return 0;
}
