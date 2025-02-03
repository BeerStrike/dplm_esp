#include "json_functions.h"
#include "cJSON.h"

char *create_scan_result_json(float range,float yaw,float pitch){
	cJSON *result_json=cJSON_CreateObject();
	cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Scan_result"));
	cJSON_AddItemToObject(result_json, "Range", cJSON_CreateNumber(range));
	cJSON_AddItemToObject(result_json, "Yaw", cJSON_CreateNumber(yaw));
	cJSON_AddItemToObject(result_json, "Pitch", cJSON_CreateNumber(pitch));
	char *str=cJSON_Print(result_json);
	cJSON_Delete(result_json);
	return str;
}
