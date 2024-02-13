#include <stdio.h>
#include <string.h>            //for handling strings
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_wifi.h"          //esp_wifi_init functions and wifi operations
#include "esp_log.h"           //for showing logs
#include "esp_event.h"         //for wifi event
#include "lwip/err.h"          //light weight ip packets error handling
#include "lwip/sys.h"          //system applications for light weight ip apps

#include "mqtt_client.h"
#include <stdint.h>
#include <stddef.h>
#include "esp_wifi.h"
#include <cJSON.h>

#include "MQ_7.h"
#include "DHT22.h"


#define ALERT_SET		(1)
#define ALERT_RESET		(0)
#define	DELAY			(15000)
#define TMP_THRES		(45)
#define	HUMID_THRES		(70)
#define CO_THRES		(100)

static uint8_t dht_alert;
static uint8_t mq7_alert;
const char *ssid = "acts";
const char *pass = "";
uint8_t MQTT_CONNECTED = 0;

uint8_t mac[6] = {0xf4, 0x96, 0x34, 0x9d, 0xe0, 0xe0}; // f4:96:34:9d:e0:e0
esp_mqtt_client_handle_t client = NULL;
SemaphoreHandle_t xSem1 = NULL;
SemaphoreHandle_t xSem2 = NULL;

float temp;
float humid;
float CO_conc;


static void wifi_event_handler(void *, esp_event_base_t, int32_t, void *);
static void wifi_connection();
static void alert_system(uint8_t);
static void mqtt_app_start(void);


static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            printf("Trying to connect with Wi-Fi\n");
            break;

        case WIFI_EVENT_STA_CONNECTED:
            printf("Wi-Fi connected\n");
            break;

        case IP_EVENT_STA_GOT_IP:
            printf("got ip: startibg MQTT Client\n");
            mqtt_app_start();
            break;

        case WIFI_EVENT_STA_DISCONNECTED:
            printf("disconnected: Retrying Wi-Fi\n");
            esp_wifi_connect();
            break;

        default:
            break;
    }
}

static void wifi_connection()
{
    // 2 - Wi-Fi Configuration Phase
    esp_netif_init();
    esp_event_loop_create_default();     // event loop                    
    esp_netif_create_default_wifi_sta(); // WiFi station                  
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //
    esp_wifi_set_mac(WIFI_IF_STA, mac);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strcpy((char*)wifi_configuration.sta.ssid, ssid);
    strcpy((char*)wifi_configuration.sta.password, pass);

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
    printf("wifi_init_softap finished. SSID:%s  password:%s", ssid, pass);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            printf("MQTT_EVENT_CONNECTED\n");
            MQTT_CONNECTED = 1;
            break;

        case MQTT_EVENT_DISCONNECTED:
            printf("MQTT_EVENT_DISCONNECTED\n");
            MQTT_CONNECTED = 0;
            break;
        case MQTT_EVENT_PUBLISHED:
            printf("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
            break;
        default:
            printf("Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    printf("Event dispatched from event loop base=%s, event_id=%ld\n", base, event_id);
    mqtt_event_handler_cb(event_data);
}


static void mqtt_app_start(void)
{
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address = {
            .uri = "mqtt://demo.thingsboard.io",
            .port = 1883,
        },
        .credentials.username = "coal_mining_token",     
    };
    
    client = esp_mqtt_client_init(&mqtt_cfg);
    
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    //xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
    //printf("MQTT Publisher_Task is up and running\n");
}

void DHT_reader_task(void *pvParameter)
{
    DHTsetgpio(GPIO_NUM_27);
    TickType_t wokenTime = xTaskGetTickCount();
    uint8_t count = 0;

	while(1) {
	
		printf("\nDHT Sensor Readings\n" );
		int ret = DHTread();
		
		DHTerrorHandler(ret);
		temp = DHTgetTemperature();
		humid = DHTgetHumidity();
		printf("Humidity %.2f %%\n", humid);
		printf("Temperature %.2f degC\n\n", temp);
		
		if (temp > TMP_THRES || humid > HUMID_THRES) {
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
		xSemaphoreGive(xSem1);
        xTaskDelayUntil(&wokenTime, pdMS_TO_TICKS(DELAY));
	}
}

void MQ7_reader_task(void *pvParameter)
{
	uint8_t count = 0;
	MQ7setgpio(GPIO_NUM_33);
    TickType_t wokenTime1 = xTaskGetTickCount();

	while(1) {
		xSemaphoreTake(xSem1, portMAX_DELAY);

		printf("\nCO gas Sensor Readings\n" );
		int con = MQ7read();
		
		MQ7errorHandler(con);

		CO_conc = MQ7getConcentration();
		printf("CO Concentration %.2f ppm\n\n", CO_conc);

		if (MQ7getConcentration() > CO_THRES) {
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
		
		xSemaphoreGive(xSem2);
        xTaskDelayUntil(&wokenTime1, pdMS_TO_TICKS(DELAY));
	}
}

void Publisher_Task(void *params)
{

    while (1)
    {
		xSemaphoreTake(xSem2, portMAX_DELAY);
		printf("\n\nPublisher Task\n");

        if (MQTT_CONNECTED) {
            printf("MQTT connected\n");

			cJSON *sensor_data = cJSON_CreateObject();
			cJSON_AddNumberToObject(sensor_data, "Temperature", temp);
			cJSON_AddNumberToObject(sensor_data, "Humidity", humid);
			cJSON_AddNumberToObject(sensor_data, "CO Concentration", CO_conc);
			char *post_data = cJSON_Print(sensor_data);

			esp_mqtt_client_publish(client, "v1/devices/me/telemetry", post_data, 0, 1, 0);
            
			printf("Data Sent to TB..\n");
			cJSON_free(post_data);
			cJSON_Delete(sensor_data);

        } else {
            printf("MQTT Not connected\n");
        }
        vTaskDelay(pdMS_TO_TICKS(DELAY));
    }
}

void app_main()
{
	nvs_flash_init();
    wifi_connection();
	printf("Wifi initialized ... \n");

	xSem1 = xSemaphoreCreateBinary();
	xSem2 = xSemaphoreCreateBinary();

    vTaskDelay(pdMS_TO_TICKS(2000));
	xTaskCreate(DHT_reader_task, "DHT_reader_task", 2048, NULL, 5, NULL );
	xTaskCreate(MQ7_reader_task, "MQ7_reader_task", 2048, NULL, 5, NULL );
	xTaskCreate(Publisher_Task, "Publisher_Task", 1024 * 5, NULL, 5, NULL);
}

void alert_system(uint8_t flag) 
{
	gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
	if ( flag == 1 || dht_alert == 1 || mq7_alert == 1)
		gpio_set_level(GPIO_NUM_32, ALERT_SET);
	else 
		gpio_set_level(GPIO_NUM_32, ALERT_RESET);
}