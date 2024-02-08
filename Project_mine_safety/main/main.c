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


void DHT_reader_task(void *pvParameter)
{
		DHTsetgpio(GPIO_NUM_27);
        TickType_t wokenTime = xTaskGetTickCount();

	while(1) {
	
		printf("DHT Sensor Readings\n" );
		int ret = DHTread();
		
		DHTerrorHandler(ret);

		printf("Humidity %.2f %%\n", DHTgetHumidity());
		printf("Temperature %.2f degC\n\n", DHTgetTemperature());
		
        xTaskDelayUntil(&wokenTime, pdMS_TO_TICKS(2000));
	}
}

void MQ7_reader_task(void *pvParameter)
{
		MQ7setgpio(GPIO_NUM_33);
        TickType_t wokenTime1 = xTaskGetTickCount();

	while(1) {
	
		printf("CO gas Sensor Readings\n" );
		int con = MQ7read();
		
		MQ7errorHandler(con);

		printf("CO Concentration %.2f ppm\n\n", MQ7getConcentration());
		
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