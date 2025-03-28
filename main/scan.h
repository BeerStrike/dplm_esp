#ifndef SCAN_H_
#define SCAN_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "globals.h"
#define	SCAN_LOG_TAG "Scaning"

struct scan_parameters {
	float length;
	float width;
	float height;
	float step;
};

TaskHandle_t scan_task_handle;

void start_scan(struct scan_parameters params);
void stop_scan();
int is_scan_active();
void scan_task(void * params);

#endif /* SCAN_H_ */
