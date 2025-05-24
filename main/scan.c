#include "scan.h"
#include "cJSON.h"
#include "laser_range.h"
#include "servo.h"
#include "room_cords_math.h"
#include "scan_cordinates_calculation.h"
#include "esp_err.h"
#include "esp_log.h"
#include "tcp_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum scan_state current_state=not_started;

esp_err_t send_error(char *error);
esp_err_t send_result(float x,float y,float h);
void scan_task(void * params);

esp_err_t send_result(float x,float y,float h){
	cJSON *result_json=cJSON_CreateObject();
	cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Scan result"));
	cJSON_AddItemToObject(result_json, "X", cJSON_CreateNumber(x));
	cJSON_AddItemToObject(result_json, "Y", cJSON_CreateNumber(y));
	cJSON_AddItemToObject(result_json, "H", cJSON_CreateNumber(h));
	char *str=cJSON_Print(result_json);
	cJSON_Delete(result_json);
	if(str==NULL){
		ESP_LOGE(SCAN_LOG_TAG,"Error create scan json");
		return ESP_FAIL;
	}
	if(send_to_pc(str)!=ESP_OK){
		ESP_LOGE(SCAN_LOG_TAG,"Error send scan json");
		return ESP_FAIL;
	}
	cJSON_free(str);
	return ESP_OK;
}

esp_err_t send_error(char *error){
	cJSON *result_json=cJSON_CreateObject();
	cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Scan error"));
	cJSON_AddItemToObject(result_json, "Error", cJSON_CreateString(error));
	char *str=cJSON_Print(result_json);
	cJSON_Delete(result_json);
	if(str==NULL){
		ESP_LOGE(SCAN_LOG_TAG,"Error create error json");
		return ESP_FAIL;
	}
	if(send_to_pc(str)!=ESP_OK){
		ESP_LOGE(SCAN_LOG_TAG,"Error send error json");
		return ESP_FAIL;
	}
	cJSON_free(str);
	return ESP_OK;
}

void scan_task(void * params){
	ESP_LOGI(SCAN_LOG_TAG,"Start scanning");
	struct scan_parameters *scprm=params;
	struct scan_status status;
	laser_range_init(1);
	laser_range_setTimeout(1000);
	//laser_range_setMeasurementTimingBudget(200000);
	if(initalize_cords_calculation(&scprm->room_cords_limiter,&scprm->scaner_pos,scprm->scaner_direction,scprm->scan_step,&status)==ESP_OK){
		while(current_state!=not_started){
			if(current_state==working){
				calculate_next_point_to_scan(&status);
				set_yaw(calculate_yaw(&status.curr_point));
				set_pitch(calculate_pitch(&status.curr_point));
				vTaskDelay(10);
				float range=((float)laser_range_readRangeSingleMillimeters())/1000;
				if(laser_range_timeoutOccurred()||laser_range_get_last_status()!=ESP_OK){
					ESP_LOGE(SCAN_LOG_TAG,"Laser range timeout");
					continue;
				}
				struct point local_point;
				calculate_real_point(&(status.curr_point),range,&local_point);
				struct point global_point;
				local_cords_to_global_cords(&scprm->scaner_pos,scprm->scaner_direction,&local_point,&global_point);
				send_result(global_point.x,global_point.y,global_point.h);
			}else
				vTaskDelay(1);
		}
	}
	free(params);
	vTaskDelete(NULL);
}



esp_err_t start_scan(struct scan_parameters params){
	if(current_state!=not_started){
	   	ESP_LOGE(SCAN_LOG_TAG,"Scan already started");
	   	return ESP_FAIL;
	}
	struct scan_parameters *p=malloc(sizeof(struct scan_parameters ));
	*p=params;
	if(xTaskCreate(scan_task,"Scan task",4096,p,1,NULL)!=pdTRUE){
	   	ESP_LOGE(SCAN_LOG_TAG,"Error create scan task");
	   	free(p);
	   	return ESP_FAIL;
	 }
   	ESP_LOGI(SCAN_LOG_TAG,"Scan started");
	current_state=working;
	return ESP_OK;
}

esp_err_t pause_scan(){
	if(current_state==not_started){
	   	ESP_LOGE(SCAN_LOG_TAG,"Scan not started");
		return ESP_FAIL;
	}else if(current_state==paused){
	   	ESP_LOGE(SCAN_LOG_TAG,"Scan already paused");
		return ESP_FAIL;
	}else{
	   	ESP_LOGI(SCAN_LOG_TAG,"Scan paused");
		current_state=paused;
	}
	return ESP_OK;
}

esp_err_t continue_scan(){
	if(current_state==not_started){
		ESP_LOGE(SCAN_LOG_TAG,"Scan not started");
		return ESP_FAIL;
	}else if(current_state==working){
	   	ESP_LOGE(SCAN_LOG_TAG,"Scan already working");
		return ESP_FAIL;
	}else{
	   	ESP_LOGI(SCAN_LOG_TAG,"Continue scanning");
		current_state=working;
	}
	return ESP_OK;
}

esp_err_t stop_scan(){
	if(current_state==not_started){
		ESP_LOGE(SCAN_LOG_TAG,"Scan not started");
		return ESP_FAIL;
	}
   	ESP_LOGI(SCAN_LOG_TAG,"Scan stopped");
	current_state=not_started;
	return ESP_OK;
}

enum scan_state get_scan_state(){
	return current_state;
}
