#include "scan.h"
#include "cJSON.h"
#include "laser_range.h"
#include "servo.h"
#include <math.h>
#include "room_cords_math.h"
#include "scan_cordinates_calculation.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tcp_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t scan_task_handle=NULL;

esp_err_t start_scan(struct scan_parameters params){
	struct scan_parameters *p=malloc(sizeof(struct scan_parameters ));
	*p=params;
	if(xTaskCreate(scan_task,"Scan task",4096,p,1,&scan_task_handle)!=pdTRUE){
	   	ESP_LOGE(SCAN_LOG_TAG,"Error create scan task");
	   	return ESP_FAIL;
	 }
	return ESP_OK;
}

esp_err_t stop_scan(){
	if(scan_task_handle){
		scan_task_handle=NULL;
		//vTaskDelete(scan_task_handle);
		return ESP_OK;
	}else{
	   	ESP_LOGE(SCAN_LOG_TAG,"scan not started");
	   	return ESP_FAIL;
	}
}
esp_err_t send_result(float x,float y,float h){
	char *payload=create_scan_result_json(x,y,h);
	if(payload){
		send_to_pc(payload);
		cJSON_free(payload);
		return ESP_OK;
	}else
		return ESP_FAIL;
}

void scan_task(void * params){
	ESP_LOGI(SCAN_LOG_TAG,"Start scanning");
	struct scan_parameters *scprm=params;
	struct scan_status status;
	laser_range_init(1);
	laser_range_setTimeout(1000);
	if(initalize_cords_calculation(scprm,&status)==ESP_OK){
		while(scan_task_handle){
			calculate_next_point_to_scan(&status);
			set_yaw(calculate_yaw(&status.curr_point));
			set_pitch(calculate_pitch(&status.curr_point));
			vTaskDelay(1);
			float range=((float)laser_range_readRangeSingleMillimeters())/1000+SCANER_RANGE_CORRECTION;
			if(laser_range_timeoutOccurred()||laser_range_last_status!=ESP_OK)
				ESP_LOGE(SCAN_LOG_TAG,"Laser range timeout");
			struct point local_point;
			calculate_real_point(&(status.curr_point),range,&local_point);
			struct point global_point;
			local_cords_to_global_cords(params,&local_point,&global_point);
			send_result(global_point.x,global_point.y,global_point.h);
		}
	}
	free(params);
	scan_task_handle=NULL;
	vTaskDelete(NULL);
}

int is_scan_active(){
	if(scan_task_handle)
		return 1;
	else
		return 0;
}

char *create_scan_result_json(float x,float y,float h){
	cJSON *result_json=cJSON_CreateObject();
	cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Scan result"));
	cJSON_AddItemToObject(result_json, "X", cJSON_CreateNumber(x));
	cJSON_AddItemToObject(result_json, "Y", cJSON_CreateNumber(y));
	cJSON_AddItemToObject(result_json, "H", cJSON_CreateNumber(h));
	char *str=cJSON_Print(result_json);
	cJSON_Delete(result_json);
	if(str==NULL)
		ESP_LOGE(SCAN_LOG_TAG,"Error create scan json");
	return str;
}

