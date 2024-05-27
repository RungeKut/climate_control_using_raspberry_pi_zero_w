#include "scd30.h"
#include "i2c.h"
#include <stdlib.h>

uint8_t cnt_measurement[8];
uint8_t mess_data[18];
uint8_t receive[4];

unsigned char ready;
unsigned int co2_value;
unsigned int temperature_temp;
unsigned int humidity_temp ;
float SCD30_ClimateData[3]; //Температура, Влажность, Концентрация CO2

/**** Start measurement ****/
void co2_sensor_measure_start(void)
{
	cnt_measurement[0]=0x00;
	cnt_measurement[1]=0x10;
	cnt_measurement[2]=0x00;
	cnt_measurement[3]=0x00;
	cnt_measurement[4]=0x81;

	HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,5,1000);
}

/***************************** Get ready Status***********************/
/***** It is essential to know when we can have data to read **********/
void co2_sensor_get_ready_status(void)
{
	cnt_measurement[0]=0x02;
	cnt_measurement[1]=0x02;

	if (HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,2,HAL_MAX_DELAY)== HAL_OK)
	{
		HAL_I2C_Master_Receive(&hi2c1,sensor_address_read,receive,3,100);
	  ready = receive[1];
	}
}

/*************** Read and display measured data ( CO2 values in PPM ) ******************/
void co2_sensor_measure_read(void)
{
	cnt_measurement[0]=0x03;
	cnt_measurement[1]=0x00;

	if (ready == 0x01 )     //always check if measurement is ready
	{
		HAL_I2C_Master_Transmit(&hi2c1,sensor_address,cnt_measurement,2,1000);
		HAL_I2C_Master_Receive(&hi2c1,sensor_address_read,mess_data,18,100);

		// cast 4 bytes to one unsigned 32 bit integer
		co2_value = (unsigned int)((((unsigned int)mess_data[0]) << 24)|
	   	                              (((unsigned int)mess_data[1]) << 16) |
	   	                                (((unsigned int)mess_data[3]) << 8) |
	   	                                  ((unsigned int)mess_data[4]));

	// cast unsigned 32 bit integer to 32 bit float

	SCD30_ClimateData[2]= *(float*)&co2_value;

	temperature_temp = (unsigned int)((((unsigned int)mess_data[6]) << 24)|
		 	                                      (((unsigned int)mess_data[7]) << 16) |
		 	                                        (((unsigned int)mess_data[9]) << 8) |
		 	                                          ((unsigned int)mess_data[10]));
	SCD30_ClimateData[0] = *(float*)&temperature_temp;

	humidity_temp = (unsigned int)((((unsigned int)mess_data[12]) << 24)|
	 	                                   (((unsigned int)mess_data[13]) << 16) |
	 	                                     (((unsigned int)mess_data[15]) << 8) |
	 	                                       ((unsigned int)mess_data[16]));
	SCD30_ClimateData[1] = *(float*)&humidity_temp;
	}
}

float *SCD30_GetData(void)
{
	co2_sensor_measure_start();
	co2_sensor_get_ready_status();
	co2_sensor_measure_read();
  return SCD30_ClimateData;
}
