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
#define MYPORT 1917

#define WIFI_SSID      ":-)"
#define WIFI_PASS      "22081609"

void json_processor(char * jsonstr);

void scan_task(void * params){
	float *Angles_array=(float *)params;
	if(params==NULL){
		printf("Error: NULL angles array\n");
		vTaskDelete(NULL);
	}
	printf("Start scanning\n");
	while(true){
		 struct sockaddr_in destAddr;
		 destAddr.sin_addr.s_addr = inet_addr("192.168.0.10");
		 destAddr.sin_family = AF_INET;
		 destAddr.sin_port = htons(1937);
		 int sct=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		 if(sct<0){
			printf("Error create udp socket\n");
		 }else{
			 for(int i=0;i<100;i++){
				 char *payload=create_scan_result_json(1.0f,Angles_array[2*i],Angles_array[2*i]+1);
				 if(payload==NULL)
					 printf("Error NULL payload\n");
				 else{
					 if (sendto(sct,payload, strlen(payload), 0, (struct sockaddr *)&destAddr, sizeof(destAddr)) < 0) {
					 	printf("Error send udp\n");
					  }else
						  printf("Udp send\n");
					 printf(payload);
					 cJSON_free(payload);
				 }
				 vTaskDelay(10);
			}
			close(sct);
		}
	}
}

void json_processor(char * jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
		if(json!=NULL){
			cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
			if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"Angles_to_scan")==0){
				cJSON *points = cJSON_GetObjectItemCaseSensitive(json, "Angles_array");
				const cJSON *point=NULL;
				int i=0;
				float *Angles_array=malloc(300*sizeof(float));
				cJSON_ArrayForEach(point, points){
					cJSON *Yaw = cJSON_GetObjectItemCaseSensitive(point, "Yaw");
					cJSON *Pitch = cJSON_GetObjectItemCaseSensitive(point, "Pitch");
					if(Yaw!=NULL&&Pitch!=NULL&&cJSON_IsNumber(Yaw)&&cJSON_IsNumber(Pitch)){
						float y=(float)Yaw->valuedouble;
						float p=(float)Pitch->valuedouble;
						Angles_array[2*i]=y;
						Angles_array[2*i+1]=p;
						i++;
					}
				}
				if(i>0){
					if(xTaskCreate(scan_task,"Scan task",4096,Angles_array,1,NULL)!=pdTRUE){
						printf("Error create scan task\n");
					}else{
						printf("Scan task created\n");
					}
				}
			}
		}
	cJSON_Delete(json);
}


void tcp_server_task(void *params){
	printf("Tcp server started\n");
    char rx_buffer[1024];
	int sct;
	struct sockaddr_in  listening_addr;
    memset(&listening_addr, 0, sizeof(listening_addr));
	listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listening_addr.sin_family = AF_INET;
	listening_addr.sin_port = htons(MYPORT);

	sct=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(sct<0){
		printf("Error create tcp socket\n");
		vTaskDelete(NULL);
	}
	if(bind(sct,(struct sockaddr *)&listening_addr,sizeof(listening_addr))<0){
		close(sct);
		printf("Error bind tcp socket\n");
		vTaskDelete(NULL);
	}
	if(listen(sct,1)<0){
		close(sct);
		printf("Error start listen tcp socket\n");
		vTaskDelete(NULL);
	}
	printf("Start listening tcp\n");
	while(true){
		struct sockaddr_in sourceAddr;
		socklen_t socklen = sizeof(sourceAddr);
		int sctaccept=accept(sct, (struct sockaddr *)&sourceAddr, &socklen);
		if(sctaccept<0)
			printf("Tcp accept error\n");
		else{
			printf("New tcp connection\n");
		    char *all_recive=malloc(1024);
		    int rcvlen=0;
		    int buffsize=1024;
			while (true) {
				 int len = recv(sctaccept, rx_buffer, sizeof(rx_buffer) - 1, 0);
				 if (len < 0) {
					  printf("Tcp recive error\n");
					  break;
				 }else if (len == 0) {
					  printf("Tcp close connection\n");
					  break;
				 }else{
					 if(rcvlen+len>buffsize){
						 all_recive=realloc(all_recive,buffsize+1024);
						 buffsize+=1024;
					 }
					 memcpy(all_recive+rcvlen,rx_buffer,len);
					 rcvlen+=len;
				 }
			}
			all_recive[rcvlen]='\0';
			printf(all_recive);
			json_processor(all_recive);
			free(all_recive);
		}
	}
}


void app_main()
{
    tcpip_adapter_init();
    esp_event_loop_create_default();

	wifi_init(WIFI_SSID,WIFI_PASS);

    if(xTaskCreate(udp_server_task,"udp server task",4096,NULL,1,NULL)!=pdTRUE){
       	printf("Udp server start error\n");
    }
    if(xTaskCreate(tcp_server_task,"tcp server task",4096,NULL,1,NULL)!=pdTRUE){
 	   printf("Tcp server start error\n");
    }

    vTaskDelete(NULL);
}
