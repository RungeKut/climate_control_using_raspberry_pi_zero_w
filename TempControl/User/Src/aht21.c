#include "aht21.h"
#include "i2c.h"
#include <stdlib.h>

uint8_t AHT_RX_Data[6];

uint32_t AHT_ADC_Raw; //Сырые данные с датчика
float AHT_ClimateData[2];
uint8_t AHT_TmpHum_Cmd = 0xAC;
#define AHT_Adress 0x38 << 1
uint8_t AHT_Switcher = 255; //Переключатель прием-0/передача-1

void AHT_Executer(void)
{
	if(AHT_Switcher)
	HAL_I2C_Master_Transmit_IT(&hi2c1, AHT_Adress, &AHT_TmpHum_Cmd, 1);
	else
	{
		HAL_I2C_Master_Receive_IT(&hi2c1, AHT_Adress, (uint8_t*)AHT_RX_Data, 6);
    
		if(~AHT_RX_Data[0] & 0x80)
		{
      /* Преобразование температуры в °C */
      AHT_ADC_Raw = (((uint32_t)AHT_RX_Data[3] & 15) << 16) | ((uint32_t)AHT_RX_Data[4] << 8) | AHT_RX_Data[5];
      AHT_ClimateData[0] = (float)(AHT_ADC_Raw * 200.00 / 1048576.00) - 50.00;
      /* Преобразование влажности в % */
      AHT_ADC_Raw = ((uint32_t)AHT_RX_Data[1] << 12) | ((uint32_t)AHT_RX_Data[2] << 4) | (AHT_RX_Data[3] >> 4);
      AHT_ClimateData[1] = (float)(AHT_ADC_Raw*100.00/1048576.00);
		}
	}
	/* Переключаем на прием */
	AHT_Switcher = ~AHT_Switcher;
}

float *AHT_GetData(void)
{
  AHT_Switcher = 255;
  AHT_Executer(); //Отправка команды
  HAL_Delay(500);
  AHT_Executer(); //Прием данных
  HAL_Delay(100);
  return AHT_ClimateData;
}
