#ifndef JSON_FUNCTIONS_H_
#define JSON_FUNCTIONS_H_
#include "cJSON.h"

char *create_scan_result_json(float range,float yaw,float pitch);
void scan_params_json_handler(cJSON * json);
void state_request_json_handler(cJSON *json);
void json_recive_processor(char *jsonstr);
#endif /* JSON_FUNCTIONS_H_ */
