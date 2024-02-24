#include <stdio.h>
#include <stdlib.h>
#include <string.h> //Requires by memset
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"
#include <esp_http_server.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include "dht/DHT11.c"
// #include "lcd/lcd.c"


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static EventGroupHandle_t s_wifi_event_group;

char html_page[] = "<!DOCTYPE HTML><html>\n"
                   "<head>\n"
                   "  <title>ESP-IDF DHT11 Web Server</title>\n"
                   "  <meta http-equiv=\"refresh\" content=\"10\">\n"
                   "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                   "  <link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">\n"
                   "  <link rel=\"icon\" href=\"data:,\">\n"
                   "  <style>\n"
                   "    html {font-family: Arial; display: inline-block; text-align: center;}\n"
                   "    p {  font-size: 1.2rem;}\n"
                   "    body {  margin: 0;}\n"
                   "    .topnav { overflow: hidden; background-color: #241d4b; color: white; font-size: 1.7rem; }\n"
                   "    .content { padding: 20px; }\n"
                   "    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }\n"
                   "    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); }\n"
                   "    .reading { font-size: 2.8rem; }\n"
                   "    .card.temperature { color: #0e7c7b; }\n"
                   "    .card.humidity { color: #17bebb; }\n"
                   "  </style>\n"
                   "</head>\n"
                   "<body>\n"
                   "  <div class=\"topnav\">\n"
                   "    <h3>ESP-IDF DHT11 WEB SERVER</h3>\n"
                   "  </div>\n"
                   "  <div class=\"content\">\n"
                   "    <div class=\"cards\">\n"
                   "      <div class=\"card temperature\">\n"
                   "        <h4><i class=\"fas fa-thermometer-half\"></i> TEMPERATURE</h4><p><span class=\"reading\">%.2f&deg;C</span></p>\n"
                   "      </div>\n"
                   "      <div class=\"card humidity\">\n"
                   "        <h4><i class=\"fas fa-tint\"></i> HUMIDITY</h4><p><span class=\"reading\">%.2f</span> &percnt;</span></p>\n"
                   "      </div>\n"
                   "    </div>\n"
                   "  </div>\n"
                   "</body>\n"
                   "</html>";
// void scan()
// {
//     wifi_scan_config_t wifi_scan_config = 
//     {
//         .ssid = 0,
//         .bssid = 0,
//         .channel = 0,
//         .show_hidden = true,
//     };

//     printf("start scanning\n");
//     esp_wifi_scan_start(&wifi_scan_config, true);
// }
// esp_wifi_init(const wifi_init_config_t *config);

#define ESP_WIFI_SSID "SAPA coffee 24h (SV)"
#define ESP_WIFI_PASSWORD "160levantho"
#define ESP_MAXIMUM_RETRY 5

static const char *TAG = "esp32 webserver";
static int s_retry_num = 0;
int wifi_connect_status = 0;

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        wifi_connect_status = 0;
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_connect_status = 1;
    }
}

void connect_wifi(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASSWORD,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASSWORD);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    vEventGroupDelete(s_wifi_event_group);
}

esp_err_t send_web_page(httpd_req_t *req)
{
    int response;
    // response = DHT11_read();
    DHT11_read();
    char response_data[sizeof(html_page) + 50];
    memset(response_data, 0, sizeof(response_data));
    sprintf(response_data, html_page, DHT11_read().temperature, DHT11_read().humidity);
    response = httpd_resp_send(req, response_data, HTTPD_RESP_USE_STRLEN);

    return response;
}

esp_err_t get_req_handler(httpd_req_t *req)
{
    return send_web_page(req);
}



httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_req_handler,
    .user_ctx = NULL};

httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
    }

    return server;
}


void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    connect_wifi();

    if (wifi_connect_status)
    {
        setup_server();
        ESP_LOGI(TAG, "Web Server is up and running\n");
    }
    else
        ESP_LOGI(TAG, "Failed to connected with Wi-Fi, check your network Credentials\n");
    while(1)
    {
        printf("Temperature is %d \n", DHT11_read().temperature);
        printf("Humidity is %d\n", DHT11_read().humidity);
        printf("Status code is %d\n", DHT11_read().status);
    }

}