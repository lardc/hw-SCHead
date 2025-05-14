#include "Measurement.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"
#include "Global.h"
#include "ZwADC.h"

uint16_t MEASURE_GetBatteryVoltage()
{
	float Offset, K, V, P2, P1;
	Int16S P0;

	Offset = (float)((Int16S)DataTable[REG_V_BAT_OFFSET]);
	K = (float)DataTable[REG_V_BAT_K] / 1000;
	V = ((float)ADC_Measure(ADC1, BAT_ADC1_CH) - Offset) * ADC_REF_VOLTAGE / ADC_RESOLUTION * K;

	// Тонкая подстройка измерения напряжения
	P2 = (float)((int16_t) DataTable[REG_V_BAT_P2]) / 1000000;
	P1 = (float)DataTable[REG_V_BAT_P1] / 1000;
	P0 = (Int16S) DataTable[REG_V_BAT_P0];

	V = V * V * P2 + V * P1 + P0;
	
	return (V > 0) ? V : 0;
}
//------------------------------------------------
