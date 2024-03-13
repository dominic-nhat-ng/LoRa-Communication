#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "../lib/DHT11.c"

#include "cJSON.h"


#define CONFIG_CS_GPIO   GPIO_NUM_5
#define CONFIG_RST_GPIO  GPIO_NUM_2
#define CONFIG_MISO_GPIO GPIO_NUM_19
#define CONFIG_MOSI_GPIO GPIO_NUM_25
#define CONFIG_SCK_GPIO  GPIO_NUM_18

#include "../lib/lora.c"
// #define 

#define SSID "Nhật"

#define PASS "1234567890"


static const char *TAG = "MQTT_TCP";

char* create_json(int temperature, int humidity)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temperature", temperature);
    cJSON_AddNumberToObject(root, "humidity", humidity);
    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_string;
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;
    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;
    case IP_EVENT_STA_GOT_IP:
        printf("WiFi got IP ... \n\n");
        break;
    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init();                    // TCP/IP initiation 					s1.1
    esp_event_loop_create_default();     // event loop 			                s1.2
    esp_netif_create_default_wifi_sta(); // WiFi station 	                    s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					                    s1.4
    // 2 - Wi-Fi Configuration Phase
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // 3 - Wi-Fi Start Phase
    esp_wifi_start();
    // 4- Wi-Fi Connect Phase
    esp_wifi_connect();
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{

    char *json_data;

   // Convert temperature to a string
    struct dht11_reading dht_data;
    // char temperature_str[6]; // Increased size to accommodate larger temperatures

    esp_mqtt_client_handle_t client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        esp_mqtt_client_subscribe(client, "v1/devices/me/telemetry", 0);

        // Initialize DHT11 sensor
        DHT11_init(GPIO_NUM_26);
        dht_data = DHT11_read();

        json_data = create_json(dht_data.temperature, dht_data.humidity);

        esp_mqtt_client_publish(client, "v1/devices/me/telemetry", json_data, 0, 1, 0);

        free(json_data);

        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (strncmp(event->topic, "v1/devices/me/rpc/request/", event->topic_len) == 0) {
            if (strncmp(event->data, "true", event->data_len) == 0) {
                gpio_set_level(GPIO_NUM_12, 1);
            } else if (strncmp(event->data, "false", event->data_len) == 0) {
                gpio_set_level(GPIO_NUM_12, 0);
            }
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

void mqtt_publish_task(void *vParameters)
{
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t)vParameters;


    while(1)
    {
        struct dht11_reading dht_data = DHT11_read();

        char *json_data = create_json(dht_data.temperature, dht_data.humidity);

        // char result[50];
        esp_mqtt_client_publish(client, "v1/devices/me/telemetry", json_data, 0, 1, 0);

        free(json_data);    

        vTaskDelay(5000/portTICK_PERIOD_MS);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    mqtt_event_handler_cb(event_data);
}
static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://Nhat:Nhat@iot.tdlogy.com",
        // .broker.address.hostname = "172.0.0.1",
        .broker.address.port = 1883,
        // .credentials.client_id = "hd4LQLPS9rCzh8COYnRc",
        // .credentials.username = "Nhat",
        // .credentials.authentication.password = "Nhat",
    };
    // while (1)
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    xTaskCreate(&mqtt_publish_task, "mqtt_publish_task", 4096, (void *)client, 5, NULL);
}

void task_tx(void *p)
{
    while(1)
    {
        struct dht11_reading dht_data = DHT11_read();

        char *json_data = create_json(dht_data.temperature, dht_data.humidity);

        // Get the length of the JSON data
        int json_data_length = strlen(json_data);

        vTaskDelay(5000/portTICK_PERIOD_MS);

        // Send the JSON data via LoRa
        lora_send_packet((uint8_t *)json_data, json_data_length);

        ESP_LOGI("CHECKING TRANSMIT DATA", "Data sent successfully");

        // Free the JSON data
        free(json_data);
    }
}

void app_main(void)
{
    // char *message = "hello";
    lora_init();
    lora_set_frequency(434e6);
    lora_enable_crc();

    //==============================//
    xTaskCreate(&task_tx, "task_tx", 2048, NULL, 5, NULL);
    
    nvs_flash_init();
    wifi_connection();

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    printf("WIFI was initiated ...........\n");
    
    mqtt_app_start();

}