#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "cJSON.h"

#include "json_functions.h"
#include "wifi_functions.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "laser_range.h"
#include "globals.h"
#include <math.h>

void scan_task(void * params){
	while(scan_params.X_step==-1){
		laser_range_askRangeSingleMillimeters();
		vTaskDelay(100);
		laser_range_readRangeSingleMillimeters();
	}
	printf("Start scanning\n");
	while(true){
		for(int nx=0;nx<60;nx++){
			for(int nz=0;nz<60;nz++){
				float yaw=atan(
						sqrt(
						(nx*scan_params.X_step)*(nx*scan_params.X_step)+
						(nz*scan_params.Z_step)*(nz*scan_params.Z_step)
						)/2
						)/M_PI*180;
				float pitch=atan((nz*scan_params.Z_step)/(nx*scan_params.X_step))/M_PI*180;
				char *payload=create_scan_result_json(1.0f,yaw,pitch);
				if(payload==NULL)
					printf("Error NULL payload\n");
				else{
					if (send(tcp_sct,payload, strlen(payload), 0) < 0) {
						printf("Error send udp\n");
					}else{
						printf("Udp send\n");
						printf(payload);
					}
					cJSON_free(payload);
					}
				vTaskDelay(1);
			}
		}
	}
}

void app_main()
{
	scan_params.X_step=-1;
	scan_params.Z_step=-1;
    tcpip_adapter_init();
    esp_event_loop_create_default();
	laser_range_init();
	wifi_init(WIFI_SSID,WIFI_PASS);

    if(xTaskCreate(udp_server_task,"Udp server task",4096,NULL,1,NULL)!=pdTRUE){
       	printf("Udp server start error\n");
    }
    if(xTaskCreate(tcp_server_task,"Tcp server task",4096,NULL,1,NULL)!=pdTRUE){
 	   printf("Tcp server start error\n");
    }
    if(xTaskCreate(scan_task,"Scan task",4096,NULL,1,NULL)!=pdTRUE){
    	printf("Error create scan task\n");
    }else{
    	printf("Scan task created\n");
    }
    vTaskDelete(NULL);
}
