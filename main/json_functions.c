#include "json_functions.h"
#include "cJSON.h"
#include <string.h>
#include "lwip/sockets.h"
#include "globals.h"
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

void json_recive_processor(char *jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
			if(json!=NULL){
				cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
				if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"Scan parameters")==0){
						cJSON *Xstp = cJSON_GetObjectItemCaseSensitive(json, "X step");
						cJSON *Zstp = cJSON_GetObjectItemCaseSensitive(json, "Z step");
						if(Xstp!=NULL&&Zstp!=NULL&&cJSON_IsNumber(Xstp)&&cJSON_IsNumber(Zstp)){
							scan_params.X_step=Xstp->valuedouble;
							scan_params.Z_step=Zstp->valuedouble;
						}
				}else if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"State request")==0){
					char json[]="{\t\"type\": \"Find response\"}";
					send(tcp_sct, json, strlen(json), 0);
				}
			}
		cJSON_Delete(json);
}
