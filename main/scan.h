#ifndef SCAN_H_
#define SCAN_H_

#include "room_cords_math.h"
#define	SCAN_LOG_TAG "Scaning"

#define SCANER_LENGTH 0.057f
#define SCANER_WIDTH 0.023f
#define SCANER_HEIGHT 0.034f
#define SCANER_RANGE_CORRECTION 0.026f

esp_err_t start_scan(struct scan_parameters params);
esp_err_t stop_scan();
esp_err_t send_result(float x,float y,float h);
int is_scan_active();
void scan_task(void * params);
char *create_scan_result_json(float x,float y,float h);


#endif /* SCAN_H_ */
