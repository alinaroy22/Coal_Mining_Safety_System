/*------------------------------------------------------------------------------
	MQ-7 CO concentration sensor driver for ESP32
	
---------------------------------------------------------------------------------*/

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#include "MQ_7.h"
#include <esp_err.h>


#define VCC		5
#define RL		10000

// == global defines =============================================

static const char *TAG = "Gas Sensor";

int MQ7gpio = 5; // my default MQ7 pin = 5
float concentration = 0;
float sensor_volt, RS_gas;

// == set the MQ7 used pin=========================================

void MQ7setgpio(int gpio)
{	
	adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11); // GPIO 33
}

// == get concentration =============================================

float MQ7getConcentration() 
{ 
	//return concentration;
	return RS_gas; 
}

// == error handler ===============================================

void MQ7errorHandler(int response)
{
	switch (response)
	{

	case MQ7_TIMEOUT_ERROR:
		ESP_LOGE(TAG, "Sensor Timeout\n");
		break;

	case MQ7_OK:
		break;

	default:
		ESP_LOGE(TAG, "Unknown error\n");
	}
}

/*----------------------------------------------------------------------------
;
;	Read analog data from MQ7
;----------------------------------------------------------------------------*/

int MQ7read()
{

	concentration = adc1_get_raw(ADC1_CHANNEL_5);
	
	sensor_volt = concentration / RL; // Measuring mq7 gas sensor voltage
	printf("Sensor_ volt: %f \n", sensor_volt);
	RS_gas = (VCC - sensor_volt) / sensor_volt; // Calculating gas concentration
	printf("Gas Concentration : %f \n", RS_gas); 

	if (concentration > 0)
		return MQ7_OK;

	else
		return MQ7_TIMEOUT_ERROR;
}