/*------------------------------------------------------------------------------
	DHT22 temperature & humidity sensor AM2302 (DHT22) driver for ESP32
	Jun 2017:	Ricardo Timmermann, new for DHT22
	Code Based on Adafruit Industries and Sam Johnston and Coffe & Beer. Please help
	to improve this code.

	This example code is in the Public Domain (or CC0 licensed, at your option.)
	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
	PLEASE KEEP THIS CODE IN LESS THAN 0XFF LINES. EACH LINE MAY CONTAIN ONE BUG !!!
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

// == global defines =============================================

static const char *TAG = "Gas Sensor";

int MQ7gpio = 5; // my default MQ7 pin = 5
float concentration = 0;

// == set the MQ7 used pin=========================================

void MQ7setgpio(int gpio)
{
	adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);          //GPIO 33
}

// == get concentration =============================================

float MQ7getConcentration() { return concentration; }

// == error handler ===============================================

void MQ7errorHandler(int response)
{
	switch (response)
	{

	case MQ7_TIMEOUT_ERROR:
		ESP_LOGE(TAG, "Sensor Timeout\n");
		break;

	case MQ7_CHECKSUM_ERROR:
		ESP_LOGE(TAG, "CheckSum error\n");
		break;

	case MQ7_OK:
		break;

	default:
		ESP_LOGE(TAG, "Unknown error\n");
	}
}

/*-------------------------------------------------------------------------------
;
;	get next state
;
;	I don't like this logic. It needs some interrupt blocking / priority
;	to ensure it runs in realtime.
;
;--------------------------------------------------------------------------------*/

int MQ7getSignalLevel(int usTimeOut, bool state)
{

	int uSec = 0;
	while (gpio_get_level(MQ7gpio) == state)
	{

		if (uSec > usTimeOut)
			return -1;

		++uSec;
		// ets_delay_us(1);		// uSec delay
		esp_rom_delay_us(1);
	}

	return uSec;
}

/*----------------------------------------------------------------------------
;
;	Read analog data from MQ7
;----------------------------------------------------------------------------*/

int MQ7read()
{

	//concentration = adc1_get_raw(ADC1_CHANNEL_5);
	concentration = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_5));
	if (concentration > 0)
		return MQ7_OK;

	else
		return MQ7_CHECKSUM_ERROR;
}