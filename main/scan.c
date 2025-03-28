#include "scan.h"
#include "cJSON.h"
#include "laser_range.h"
#include "servo.h"
#include <math.h>
#include "globals.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "json_functions.h"
#include "esp_err.h"
#include "esp_log.h"


void start_scan(struct scan_parameters params){
	struct scan_parameters *p=malloc(sizeof(struct scan_parameters ));
	*p=params;
	if(xTaskCreate(scan_task,"Scan task",4096,p,1,&scan_task_handle)!=pdTRUE){
	   	ESP_LOGE(SCAN_LOG_TAG,"Error create scan task");
	 }
}

void scan_task(void * params){
	ESP_LOGI(SCAN_LOG_TAG,"Start scanning");
	struct scan_parameters *scprm=params;
	float step=scprm->step;
	float l=scprm->length;
	float w=scprm->width;
	float h=scprm->height;
	char xdirect=0;
	char zdirect=0;
	int nx=0;
	int nz=0;
	float yaw=0;
	float pitch=0;
	free(params);
	while(scan_task_handle){
		if (nx >= 40 || nx <= 0) {
			xdirect = !xdirect;
			if (nz >= 40 || nz <= 0) {
				zdirect = !zdirect;
			}
			if (zdirect)
				nz++;
			else
				nz--;
		}
		if (xdirect)
			nx++;
		else
			nx--;
		if (nz == 20 && xdirect == 0 && zdirect == 0 && nx >= 20)
			continue;
		if (nx <= 20 && nz <= 20) {
			yaw = atan(sqrt((nx * step) * (nx * step) + (nz * step) * (nz * step)) / h) / M_PI * 180;
			pitch = atan((nz * step) / (nx * step)) / M_PI * 180;
		}
		else if (nx > 20 && nz <= 20) {
			yaw = (atan(sqrt(l * l + (nz * step) * (nz * step))/((40 - nx) * step))) / M_PI * 180;
			pitch = atan((nz * step) / l) / M_PI * 180;
		}
		else if (nx <= 20 && nz > 20) {
			yaw = (M_PI / 2 - atan((40 - nz) * step / sqrt(w * w + (nx * step) * (nx * step)))) / M_PI * 180;
			pitch = atan(w / (nx * step)) / M_PI * 180;
		}
		else
			continue;
		set_yaw(yaw);
		set_pitch(pitch);
		float range=laser_range_readRangeSingleMillimeters();
		char *payload=create_scan_result_json(range,yaw,pitch);
		if(payload==NULL)
			printf("Error NULL payload\n");
		else{
			if (send(tcp_sct,payload, strlen(payload), 0) < 0) {
				printf("Error send udp\n");
				scan_task_handle=NULL;
			}else{
				printf("Udp send\n");
			}
			cJSON_free(payload);
		}
	}
	vTaskDelete(NULL);
}

void stop_scan(){
	scan_task_handle=NULL;
}

int is_scan_active(){
	if(scan_task_handle)
		return 1;
	else
		return 0;
}

