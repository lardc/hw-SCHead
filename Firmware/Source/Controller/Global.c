﻿#include "LocalDefs.h"
#include "Global.h"
#include "DataTable.h"

//Переменные
volatile Int64U CONTROL_TimeCounter = 0;
Int64U LED_BlinkTimeCounter = 0;
volatile uint16_t ADC_BUF[ADC_BUFF_LENGTH];
