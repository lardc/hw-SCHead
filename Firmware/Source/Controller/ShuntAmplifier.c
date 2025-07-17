#include "ShuntAmplifier.h"


//Переменные
float Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D1;
float K_ShuntAmplifier = 1.0f;
float B_ShuntAmplifier = 0.0f;
//


//------------------------------------------------------------------------------
void Set_K_ShuntAmplifier1(float Ky)
{
  DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER1_D1*1000);
  ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N1);
  Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D1;
  K_ShuntAmplifier = 0.001f * DataTable[REG_CS1_K1_I_CAL];
  B_ShuntAmplifier = (Int16S)DataTable[REG_CS1_B1_I_CAL];

  if(Ky>=SHUNT_AMPLIFIER1_D2)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER1_D2*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N2);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D2;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS1_K2_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS1_B2_I_CAL];
  }

  if(Ky>=SHUNT_AMPLIFIER1_D3)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER1_D3*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N3);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D3;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS1_K3_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS1_B3_I_CAL];
  }

  //Не используются
  /*if(Ky>=SHUNT_AMPLIFIER1_D4)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER1_D4*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N4);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D4;
    K_ShuntAmplifier = DataTable[REG_CS1_K4_I_CAL];
    B_ShuntAmplifier = DataTable[REG_CS1_B4_I_CAL];
  }

  if(Ky>=SHUNT_AMPLIFIER1_D5)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER1_D5*100);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N5);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER1_D5;
    K_ShuntAmplifier = DataTable[REG_CS1_K5_I_CAL];
    B_ShuntAmplifier = DataTable[REG_CS1_B5_I_CAL];
  } */
  //
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Set_K_ShuntAmplifier2(float Ky)
{
  DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER2_D1*1000);
  ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N1);
  Ky_ShuntAmplifier = SHUNT_AMPLIFIER2_D1;
  K_ShuntAmplifier = 0.001f * DataTable[REG_CS2_K1_I_CAL];
  B_ShuntAmplifier = (Int16S)DataTable[REG_CS2_B1_I_CAL];

  if(Ky>=SHUNT_AMPLIFIER2_D2)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER2_D2*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N2);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER2_D2;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS2_K2_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS2_B2_I_CAL];
  }

  if(Ky>=SHUNT_AMPLIFIER2_D3)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER2_D3*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N3);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER2_D3;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS2_K3_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS2_B3_I_CAL];
  }

  if(Ky>=SHUNT_AMPLIFIER2_D4)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER2_D4*1000);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N4);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER2_D4;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS2_K4_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS2_B4_I_CAL];
  }

  if(Ky>=SHUNT_AMPLIFIER2_D5)
  {
    DataTable[REG_INFO_K_SHUNT_AMP] = (uint16_t)(SHUNT_AMPLIFIER2_D5*100);
    ShuntAmplyfierConfig(SHUNT_AMPLIFIER_N5);
    Ky_ShuntAmplifier = SHUNT_AMPLIFIER2_D5;
    K_ShuntAmplifier = 0.001f * DataTable[REG_CS2_K5_I_CAL];
    B_ShuntAmplifier = (Int16S)DataTable[REG_CS2_B5_I_CAL];
  }
  //
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void ShuntAmplyfierConfig(uint16_t Ky_Number)
{
  SPI_WriteByte(SPI1, Ky_Number);
  SPI1_CS_1;
  SPI1_CS_0;
}
//------------------------------------------------------------------------------
