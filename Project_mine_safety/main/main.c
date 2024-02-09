#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "MQ_7.h"
#include "DHT22.h"

#define ALERT_SET		(1)
#define ALERT_RESET		(0)

static uint8_t dht_alert;
static uint8_t mq7_alert;

void alert_system(uint8_t);

void DHT_reader_task(void *pvParameter)
{
		DHTsetgpio(GPIO_NUM_27);
        TickType_t wokenTime = xTaskGetTickCount();
		uint8_t count = 0;

	while(1) {
	
		printf("DHT Sensor Readings\n" );
		int ret = DHTread();
		
		DHTerrorHandler(ret);

		printf("Humidity %.2f %%\n", DHTgetHumidity());
		printf("Temperature %.2f degC\n\n", DHTgetTemperature());
		
		if (DHTgetTemperature() > 25) {
			count++;
			if (count > 5) {
				dht_alert = 1;
				alert_system(ALERT_SET);
			}
		} else {
			count = 0;
			dht_alert = 0;
			alert_system(ALERT_RESET);
		}
        xTaskDelayUntil(&wokenTime, pdMS_TO_TICKS(2000));
	}
}

void MQ7_reader_task(void *pvParameter)
{
	uint8_t count = 0;
	MQ7setgpio(GPIO_NUM_33);
    TickType_t wokenTime1 = xTaskGetTickCount();

	while(1) {
	
		printf("CO gas Sensor Readings\n" );
		int con = MQ7read();
		
		MQ7errorHandler(con);

		printf("CO Concentration %.2f ppm\n\n", MQ7getConcentration());

		if (MQ7getConcentration() > 1500) {
			count++;
			if (count > 5) {
				mq7_alert = 1;
				alert_system(ALERT_SET);
			}
		} else {
			count = 0;
			mq7_alert = 0;
			alert_system(ALERT_RESET);
		}
		
        xTaskDelayUntil(&wokenTime1, pdMS_TO_TICKS(2000));
	}
}

void app_main()
{
	nvs_flash_init();
        //vTaskDelay(2000 / portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(2000));
	xTaskCreate(&DHT_reader_task, "DHT_reader_task", 2048, NULL, 5, NULL );
	xTaskCreate(&MQ7_reader_task, "MQ7_reader_task", 2048, NULL, 5, NULL );
}

void alert_system(uint8_t flag) 
{
	gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
	if ( flag == 1 || dht_alert == 1 || mq7_alert == 1)
		gpio_set_level(GPIO_NUM_32, ALERT_SET);
	else 
		gpio_set_level(GPIO_NUM_32, ALERT_RESET);
}