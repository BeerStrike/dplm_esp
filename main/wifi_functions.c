#include "wifi_functions.h"
#include <string.h>
#include "esp_log.h"
#include "globals.h"
void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi started");
        esp_wifi_connect();
    }else if(event_base==WIFI_EVENT&&event_id==WIFI_EVENT_STA_DISCONNECTED){
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi disconnected");
        esp_wifi_connect();
    }
}

void wifi_init(char *ssid,char *pass)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    //esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
   wifi_config_t wifi_config;
    /*

    wifi_config_t wifi_config = {
            .sta = {
                .ssid = wifi_ssid,
                .password = wifi_pass
            },
        };
        	*/
	strcpy((char *)wifi_config.sta.ssid,ssid);
	strcpy((char *)wifi_config.sta.password,pass);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}
