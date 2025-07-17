#ifndef __SHUNT_AMPLIFIER_H
#define __SHUNT_AMPLIFIER_H

#include "ZwSPI.h"
#include "DataTable.h"
#include "DeviceObjectDictionary.h"

//
#define SHUNT_AMPLIFIER1_D1                     0.997f                          //ShuntAmplifier Ky=1.03
#define SHUNT_AMPLIFIER1_D2                     3.227f                           //ShuntAmplifier Ky=3.27
#define SHUNT_AMPLIFIER1_D3                     10.944f                           //ShuntAmplifier Ky=9.89
#define SHUNT_AMPLIFIER1_D4                     32.700f                          //ShuntAmplifier Ky=31.58
#define SHUNT_AMPLIFIER1_D5                     85.000f                          //ShuntAmplifier Ky=81.82
//
#define SHUNT_AMPLIFIER2_D1                     0.997f                           //ShuntAmplifier Ky=1.03
#define SHUNT_AMPLIFIER2_D2                     3.212f                           //ShuntAmplifier Ky=3.27
#define SHUNT_AMPLIFIER2_D3                     10.944f                           //ShuntAmplifier Ky=9.89
#define SHUNT_AMPLIFIER2_D4                     32.750f                          //ShuntAmplifier Ky=31.58
#define SHUNT_AMPLIFIER2_D5                     85.522f                          //ShuntAmplifier Ky=81.82
//
#define SHUNT_AMPLIFIER_N1                      0                               //Код диапазона ShuntAmplifier Ky=1
#define SHUNT_AMPLIFIER_N2                      2                               //Код диапазона ShuntAmplifier Ky=3.125
#define SHUNT_AMPLIFIER_N3                      4                               //Код диапазона ShuntAmplifier Ky=10.89
#define SHUNT_AMPLIFIER_N4                      8                               //Код диапазона ShuntAmplifier Ky=32.50
#define SHUNT_AMPLIFIER_N5                      16                              //Код диапазона ShuntAmplifier Ky=81.94
//

//
#define K1_I_CAL_DEFAULT                        1000
#define B1_I_CAL_DEFAULT                        0
#define K2_I_CAL_DEFAULT                        1015
#define B2_I_CAL_DEFAULT                        23
#define K3_I_CAL_DEFAULT                        1021
#define B3_I_CAL_DEFAULT                        53
#define K4_I_CAL_DEFAULT                        1000
#define B4_I_CAL_DEFAULT                        0
#define K5_I_CAL_DEFAULT                        1000
#define B5_I_CAL_DEFAULT                        0

//Переменные
extern float Ky_ShuntAmplifier, K_ShuntAmplifier, B_ShuntAmplifier;

//Функции
void ShuntAmplyfierConfig(uint16_t Ky_Number);
void Set_K_ShuntAmplifier1(float Ky);
void Set_K_ShuntAmplifier2(float Ky);
//

#endif // __SHUNT_AMPLIFIER_H
