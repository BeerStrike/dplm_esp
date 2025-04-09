#include "json_functions.h"
#include <string.h>
#include "lwip/sockets.h"
#include "globals.h"
#include "scan.h"
#include "esp_log.h"
#include "flash.h"
#include "wifi_functions.h"

char *create_scan_result_json(float range,float yaw,float pitch){
	cJSON *result_json=cJSON_CreateObject();
	cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Scan result"));
	cJSON_AddItemToObject(result_json, "Range", cJSON_CreateNumber(range));
	cJSON_AddItemToObject(result_json, "Yaw", cJSON_CreateNumber(yaw));
	cJSON_AddItemToObject(result_json, "Pitch", cJSON_CreateNumber(pitch));
	char *str=cJSON_Print(result_json);
	cJSON_Delete(result_json);
	return str;
}

void scan_params_json_handler(cJSON * json){
	struct scan_parameters params;
	cJSON *length = cJSON_GetObjectItemCaseSensitive(json, "Length");
	cJSON *width = cJSON_GetObjectItemCaseSensitive(json, "Width");
	cJSON *height = cJSON_GetObjectItemCaseSensitive(json, "Height");
	cJSON *step = cJSON_GetObjectItemCaseSensitive(json, "Step");
	if(length!=NULL&&width!=NULL&&height!=NULL&&step!=NULL
			&&cJSON_IsNumber(length)&&cJSON_IsNumber(width)&&cJSON_IsNumber(height)&&cJSON_IsNumber(step)){
		params.length=length->valuedouble;
		params.width=width->valuedouble;
		params.height=height->valuedouble;
		params.step=step->valuedouble;
		start_scan(params);
	}
}

void state_request_json_handler(cJSON *json){
	cJSON *response_json=cJSON_CreateObject();
	cJSON_AddItemToObject(response_json, "Type", cJSON_CreateString("State response"));
	if(is_scan_active())
		cJSON_AddItemToObject(response_json, "State", cJSON_CreateString("Working"));
	else
		cJSON_AddItemToObject(response_json, "State", cJSON_CreateString("Wait for params"));
	char *jsonstr=cJSON_Print(response_json);
	printf(jsonstr);
	send(tcp_sct, jsonstr, strlen(jsonstr), 0);
	cJSON_free(json);
	cJSON_Delete(response_json);

}
void json_recive_processor(char *jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
	if(json!=NULL){
		cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
		if(type!=NULL&&type->valuestring!=NULL){
			if(strcmp(type->valuestring,"Scan parameters")==0)
				scan_params_json_handler(json);
			else if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"State request")==0)
				state_request_json_handler(json);
		}
		cJSON_Delete(json);
	}
}

char *create_settings_json(){
	cJSON *json = cJSON_CreateObject();
	//load_settings_from_flash();
	cJSON_AddItemToObject(json, "Type", cJSON_CreateString("Settings response"));
	cJSON_AddItemToObject(json, "Wi-fi SSID", cJSON_CreateString(wifi_ssid));
	cJSON_AddItemToObject(json, "Wi-fi password", cJSON_CreateString(wifi_pass));
	cJSON_AddItemToObject(json, "Scaner name", cJSON_CreateString(scaner_name));
	char *str=cJSON_Print(json);
	cJSON_Delete(json);
	return str;
}

void set_settings_from_json(cJSON *json){
	cJSON *wifi_ssid_json = cJSON_GetObjectItemCaseSensitive(json, "Wi-fi SSID");
	cJSON *wifi_pass_json = cJSON_GetObjectItemCaseSensitive(json, "Wi-fi password");
	cJSON *scaner_name_json = cJSON_GetObjectItemCaseSensitive(json, "Scaner name");
	if(wifi_ssid_json!=NULL&&wifi_pass_json!=NULL&&scaner_name_json!=NULL
		&&cJSON_IsString(wifi_ssid_json)&&cJSON_IsString(wifi_pass_json)&&cJSON_IsString(scaner_name_json)&&
		wifi_ssid_json->valuestring && wifi_pass_json->valuestring && scaner_name_json->valuestring
		){
		strcpy(wifi_ssid,wifi_ssid_json->valuestring);
		strcpy(wifi_pass,wifi_pass_json->valuestring);
		strcpy(scaner_name,scaner_name_json->valuestring);
		save_settings_to_flash();
		wifi_init();
	}

}
char *uart_setup_json_handler(char * jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
	char * ret=NULL;
	if(json!=NULL){
		cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
		if(type!=NULL&&type->valuestring!=NULL){
			if(strcmp(type->valuestring,"Request settings")==0){
				ret=create_settings_json();
			}else if(strcmp(type->valuestring,"Set settings")==0){
				set_settings_from_json(json);
			}
		}
		cJSON_Delete(json);
	}
	return ret;
}

