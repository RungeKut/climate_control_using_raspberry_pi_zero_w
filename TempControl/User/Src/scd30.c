#include "scd30.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "bt_usart.h"

uint8_t cnt_measurement[8];
uint8_t mess_data[18];
uint8_t receive[4];

unsigned char ready;
uint32_t co2_value;
uint32_t temperature_temp;
uint32_t humidity_temp ;
float SCD30_ClimateData[3]; //Температура, Влажность, Концентрация CO2

float controlValue = 0;
uint8_t controlValueCount = 0;

/**** Start measurement ****/
void co2_sensor_measure_start(void)
{
	cnt_measurement[0]=0x00;
	cnt_measurement[1]=0x10;
	cnt_measurement[2]=0x00;
	cnt_measurement[3]=0x00;
	cnt_measurement[4]=0x81;
	HAL_StatusTypeDef status;

	status = HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,5,HAL_MAX_DELAY);
	if (status != HAL_OK){
			char buf[35] = {0}, *bufPos = buf;
			bufPos += sprintf(bufPos, "Err: scd30 measure start ");
			bufPos += sprintf(bufPos, "%d", status);
			bufPos += sprintf(bufPos, "\r\n");
			Uart_sendstring(buf);
		}
}

/***************************** Get ready Status***********************/
/***** It is essential to know when we can have data to read **********/
void co2_sensor_get_ready_status(void)
{
	cnt_measurement[0]=0x02;
	cnt_measurement[1]=0x02;
	HAL_StatusTypeDef status;

	status = HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,2,HAL_MAX_DELAY);
	if (status == HAL_OK)
	{
		status = HAL_I2C_Master_Receive(&hi2c1,sensor_address_read,receive,3,100);
	  ready = receive[1];
		if (ready != 0x01 ){
			char buf[35] = {0}, *bufPos = buf;
			bufPos += sprintf(bufPos, "Err: scd30 Ready status ");
			bufPos += sprintf(bufPos, "%d", ready);
			bufPos += sprintf(bufPos, "\r\n");
			Uart_sendstring(buf);
			HAL_Delay(1000);
			co2_sensor_get_ready_status();
		}
	}
}

/*************** Read and display measured data ( CO2 values in PPM ) ******************/
void co2_sensor_measure_read(void)
{
	cnt_measurement[0]=0x03;
	cnt_measurement[1]=0x00;
	HAL_StatusTypeDef status;

	if (ready == 0x01 )     //always check if measurement is ready
	{
		status = HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,2,1000);
		if (status != HAL_OK){
			char buf[35] = {0}, *bufPos = buf;
			bufPos += sprintf(bufPos, "Err: scd30 I2C Transmit status ");
			bufPos += sprintf(bufPos, "%d", status);
			bufPos += sprintf(bufPos, "\r\n");
			Uart_sendstring(buf);
		}
		status = HAL_I2C_Master_Receive(&hi2c1,sensor_address_read,mess_data,18,1000);
		if (status != HAL_OK){
			char buf[35] = {0}, *bufPos = buf;
			bufPos += sprintf(bufPos, "Err: scd30 I2C Receive status ");
			bufPos += sprintf(bufPos, "%d", status);
			bufPos += sprintf(bufPos, "\r\n");
			Uart_sendstring(buf);
		}

		co2_value = (mess_data[0] << 24)+(mess_data[1] << 16)+(mess_data[3] << 8)+(mess_data[4]);
		SCD30_ClimateData[2]= *(float*)&co2_value;

		temperature_temp = (mess_data[6] << 24)+(mess_data[7] << 16)+(mess_data[9] << 8)+(mess_data[10]);
		SCD30_ClimateData[0] = *(float*)&temperature_temp;

		humidity_temp = (mess_data[12] << 24)+(mess_data[13] << 16)+(mess_data[15] << 8)+(mess_data[16]);
		SCD30_ClimateData[1] = *(float*)&humidity_temp;
		
		if (controlValue == SCD30_ClimateData[1])
		{
			controlValueCount++;
			if (controlValueCount > 100)
			{
				Error_Handler();
			}
		}
		else
		{
			controlValue = SCD30_ClimateData[1];
			controlValueCount = 0;
		}
	}
}

float *SCD30_GetData(void)
{
	co2_sensor_measure_start();
	co2_sensor_get_ready_status();
	co2_sensor_measure_read();
  return SCD30_ClimateData;
}
