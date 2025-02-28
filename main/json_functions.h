#ifndef JSON_FUNCTIONS_H_
#define JSON_FUNCTIONS_H_

char *create_scan_result_json(float range,float yaw,float pitch);
void json_recive_processor(char *jsonstr);
#endif /* JSON_FUNCTIONS_H_ */
