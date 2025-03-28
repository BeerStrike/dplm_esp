#include "json_functions.h"
#include <string.h>
#include "lwip/sockets.h"
#include "globals.h"
#include "scan.h"

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
		if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"Scan parameters")==0){
			scan_params_json_handler(json);
		}else if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"State request")==0){
			state_request_json_handler(json);
		}
	}
	cJSON_Delete(json);
}
