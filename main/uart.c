#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "uart.h"
#include "driver/uart.h"
#include <string.h>
#include "cJSON.h"
#include "flash.h"
#include "wifi.h"
TaskHandle_t uart_task_handle=NULL;
QueueHandle_t uart_queue=NULL;

void uart_event_task();
void uart_json_handler(char * jsonstr);
void set_settings_from_json(cJSON *json);
esp_err_t send_settings_json();

void uart_json_handler(char * jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
	if(json!=NULL){
		cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
		if(type!=NULL&&type->valuestring!=NULL){
			if(strcmp(type->valuestring,"Request settings")==0){
				send_settings_json();
			}else if(strcmp(type->valuestring,"Set settings")==0){
				set_settings_from_json(json);
			}
		}
		cJSON_Delete(json);
	}
}

void set_settings_from_json(cJSON *json){
	cJSON *wifi_ssid_json = cJSON_GetObjectItemCaseSensitive(json, "Wi-fi SSID");
	cJSON *wifi_pass_json = cJSON_GetObjectItemCaseSensitive(json, "Wi-fi password");
	if(wifi_ssid_json!=NULL&&wifi_pass_json!=NULL&&cJSON_IsString(wifi_ssid_json)&&cJSON_IsString(wifi_pass_json)){
		char wifi_ssid[32];
		char wifi_pass[64];
		strcpy(wifi_ssid,wifi_ssid_json->valuestring);
		strcpy(wifi_pass,wifi_pass_json->valuestring);
		save_wifi_settings_to_flash(wifi_ssid,wifi_pass);
		wifi_stop();
		wifi_start(wifi_ssid,wifi_pass);
	}
	cJSON *scaner_name_json = cJSON_GetObjectItemCaseSensitive(json, "Scaner name");
	if(scaner_name_json!=NULL&&cJSON_IsString(scaner_name_json)){
		char scaner_name[64];
		strcpy(scaner_name,scaner_name_json->valuestring);
		save_scaner_name_to_flash(scaner_name);
	}
	cJSON *scaner_port_json = cJSON_GetObjectItemCaseSensitive(json, "Scaner port");
	if(scaner_port_json!=NULL&&cJSON_IsNumber(scaner_port_json)){
		int16_t port=scaner_port_json->valueint;
		save_port_to_flash(port);
	}
	cJSON *range_calibration_json = cJSON_GetObjectItemCaseSensitive(json, "Range calibration");
	if(range_calibration_json!=NULL&&cJSON_IsNumber(range_calibration_json)){
		int16_t range_calibration=range_calibration_json->valueint;
		save_range_calibration_to_flash(range_calibration);
	}
}
esp_err_t send_settings_json(){
	cJSON *json = cJSON_CreateObject();
	char wifi_ssid[32];
	char wifi_pass[64];
	load_wifi_settings_from_flash(wifi_ssid,wifi_pass);
	char scaner_name[64];
	load_scaner_name_from_flash(scaner_name);
	int16_t scaner_port;
	load_port_from_flash(&scaner_port);
	int16_t range_calibration;
	load_range_calibration_from_flash(&range_calibration);
	cJSON_AddItemToObject(json, "Type", cJSON_CreateString("Settings response"));
	cJSON_AddItemToObject(json, "Wi-fi SSID", cJSON_CreateString(wifi_ssid));
	cJSON_AddItemToObject(json, "Wi-fi password", cJSON_CreateString(wifi_pass));
	cJSON_AddItemToObject(json, "Scaner name", cJSON_CreateString(scaner_name));
	cJSON_AddItemToObject(json, "Scaner port", cJSON_CreateNumber(scaner_port));
	cJSON_AddItemToObject(json, "Range calibration", cJSON_CreateNumber(range_calibration));
	char *str=cJSON_Print(json);
	cJSON_Delete(json);
	if(uart_write_bytes(UART_NUM_0,str,strlen(str))==-1){
		 ESP_LOGE(UART_LOG_TAG,"Error write settings");
		 return ESP_FAIL;
	}
	cJSON_free(str);
	return ESP_OK;
}

esp_err_t uart_init(){
	if(uart_task_handle){
		ESP_LOGE(UART_LOG_TAG,"uart already initalized");
		return ESP_FAIL;
	}
	esp_err_t err;
	 uart_config_t uart_config = {
			.baud_rate = 78800,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	 };
	 err=uart_param_config(UART_NUM_0, &uart_config);
	 if(err!=ESP_OK){
		 ESP_LOGE(UART_LOG_TAG,"Error param config: %d",err);
		 return err;
	 }
	 err=uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart_queue, 0);
	 if(err!=ESP_OK){
		 ESP_LOGE(UART_LOG_TAG,"Error driver install: %d",err);
		 return err;
	 }
	 if(xTaskCreate(uart_event_task, "uart event task", 2048, NULL, 12, NULL)!=pdTRUE){
		 ESP_LOGE(UART_LOG_TAG,"Error start uart task: %d",err);
		 return ESP_FAIL;
	 }
	 return ESP_OK;
}

void uart_event_task(){
	uart_event_t event;
	int bracket_counter=0;
	uint8_t * json_start=NULL;
	uint8_t * json_end=NULL;
    uint8_t *rx_buff = (uint8_t *) malloc(RX_BUF_SIZE);
	uint8_t *rx_buff_pointer=rx_buff;
	while(1){
		if (xQueueReceive(uart_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
            	case UART_DATA:
            		 uart_read_bytes(UART_NUM_0, rx_buff_pointer, event.size, portMAX_DELAY);
					 ESP_LOGI(UART_LOG_TAG,"Recive json: %s",rx_buff);
            		 for(int i=0;i<event.size;i++){
            			 if(*((char *)rx_buff_pointer)=='{'){
            				 if(bracket_counter==0)
            					 json_start=rx_buff_pointer;
            				 bracket_counter++;
            			 }
            			 if(*((char *)rx_buff_pointer)=='}'){
            			      bracket_counter--;
            			      if(bracket_counter==0)
            			    	  json_end=rx_buff_pointer+1;
            			 }
            			 rx_buff_pointer++;
            		 }
            		 if(json_start&&json_end){
						 *((char *)json_end)='\0';
						 ESP_LOGI(UART_LOG_TAG,"Recive json: %s",json_start);
						 uart_json_handler((char *)json_start);
						 rx_buff_pointer=rx_buff;
						 bracket_counter=0;
						 json_start=NULL;
						 json_end=NULL;
            		 }
            		  break;
            	default:
            		break;
            }
		}
	}
    free(rx_buff);
	vTaskDelete(NULL);
}



