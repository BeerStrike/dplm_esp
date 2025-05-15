#include "wifi.h"

#include <string.h>
#include "esp_log.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "esp_wifi.h"
#include "flash.h"

void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi started");
        esp_wifi_connect();
    }if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP) {
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi stoped");
    }else if(event_base==WIFI_EVENT&&event_id==WIFI_EVENT_STA_CONNECTED){
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi connected");
    	int16_t port;
    	if(load_port_from_flash(&port)==ESP_OK){
    		start_udp_server(port);
    		start_tcp_server(port);
    	}
    }else if(event_base==WIFI_EVENT&&event_id==WIFI_EVENT_STA_DISCONNECTED){
    	ESP_LOGI(WIFI_LOG_TAG,"Wifi disconnected");
        esp_wifi_connect();
        stop_udp_server();
        stop_tcp_server();
    }
}

esp_err_t wifi_init()
{
	tcpip_adapter_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err;
    err=esp_event_loop_create_default();
    if(err!=ESP_OK){
    	ESP_LOGE(WIFI_LOG_TAG,"Error create event loop");
    	return err;
    }
    err=esp_wifi_init(&cfg);
    if(err!=ESP_OK){
    	ESP_LOGE(WIFI_LOG_TAG,"Error wifi init");
    	return err;
    }
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    return ESP_OK;
}

esp_err_t wifi_start(char *wifi_ssid,char *wifi_pass){
	wifi_config_t wifi_config = {
	       .sta = {
	    		   .ssid = "",
	                .password = ""
	            },
	        };

	strcpy((char *)wifi_config.sta.ssid,wifi_ssid);
	strcpy((char *)wifi_config.sta.password,wifi_pass);
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
	esp_err_t err= esp_wifi_start();
	if(err!=ESP_OK){
		ESP_LOGE(WIFI_LOG_TAG,"Error wifi start");
		return err;
	}
	return ESP_OK;
}

esp_err_t wifi_stop(){
	esp_err_t err=esp_wifi_disconnect();
	if(err!=ESP_OK){
		ESP_LOGE(WIFI_LOG_TAG,"Error wifi disconnect");
		return err;
	}
	err= esp_wifi_stop();
	if(err!=ESP_OK){
		ESP_LOGE(WIFI_LOG_TAG,"Error wifi stop");
		return err;
	}
	return ESP_OK;
}

