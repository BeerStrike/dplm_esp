#include "udp_server.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "cJSON.h"

TaskHandle_t udp_server_task_handle=NULL;
int udp_sct=-1;

void udp_server_task(void *params);
esp_err_t udp_server_init(int port);

esp_err_t start_udp_server(int port){
	if(udp_server_task_handle!=NULL){
		ESP_LOGE(UDP_SERVER_LOG_TAG,"Udp server already started");
		return ESP_FAIL;
	}
	if(udp_server_init(port)!=ESP_OK){
		ESP_LOGE(UDP_SERVER_LOG_TAG,"Udp server init error");
		return ESP_FAIL;
	}
	if(xTaskCreate(udp_server_task,"Udp server task",4096,&udp_server_task_handle,1,NULL)!=pdTRUE){
		ESP_LOGE(UDP_SERVER_LOG_TAG,"Udp server start error");
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t udp_server_init(int port){
	struct sockaddr_in  listening_addr;
	memset(&listening_addr, 0, sizeof(listening_addr));
	listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listening_addr.sin_family = AF_INET;
	listening_addr.sin_port = htons(port);
	udp_sct=socket(AF_INET,SOCK_DGRAM ,IPPROTO_UDP);
	if(udp_sct<0){
		ESP_LOGI(UDP_SERVER_LOG_TAG,"Error create UDP socket");
		return ESP_FAIL;
	}
	if(bind(udp_sct,(struct sockaddr *)&listening_addr,sizeof(listening_addr))<0){
		close(udp_sct);
		ESP_LOGE(UDP_SERVER_LOG_TAG,"Error bind UDP socket");
		return ESP_FAIL;
	}
	return ESP_OK;
}

void udp_server_task(void *params){
    ESP_LOGI(UDP_SERVER_LOG_TAG,"UDP server started");
    char rx_buffer[UDP_SERVER_RX_BUFFER_SIZE];
    struct sockaddr_in sourceAddr;
    socklen_t socklen = sizeof(sourceAddr);
    ESP_LOGI(UDP_SERVER_LOG_TAG,"Start listening UDP");
	while(true){
		int len =recvfrom(udp_sct, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&sourceAddr, &socklen);
		if (len < 0) {
			ESP_LOGI(UDP_SERVER_LOG_TAG,"UDP recive error");
		}else{
			ESP_LOGI(UDP_SERVER_LOG_TAG,"Recive UDP");
			cJSON *json = cJSON_Parse(rx_buffer);
			if(json!=NULL){
				cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
				if(type!=NULL&&type->valuestring!=NULL){
					if(strcmp(type->valuestring,"Find request")==0){
						cJSON *result_json=cJSON_CreateObject();
						cJSON_AddItemToObject(result_json, "Type", cJSON_CreateString("Find response"));
						char *str=cJSON_Print(result_json);
						cJSON_Delete(result_json);
						if(sendto(udp_sct, str, strlen(str), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr))<0)
							ESP_LOGE(UDP_SERVER_LOG_TAG,"UDP send error");
						else
							ESP_LOGI(UDP_SERVER_LOG_TAG,"Send UDP: %s",str);
					}else
						ESP_LOGE(UDP_SERVER_LOG_TAG,"Unknown message type");
				}else
					ESP_LOGE(UDP_SERVER_LOG_TAG,"Unknown json structure");
				cJSON_Delete(json);
			}else
				ESP_LOGE(UDP_SERVER_LOG_TAG,"Error parse json");
		}
	}
}

esp_err_t stop_udp_server(){
	if(udp_server_task_handle==NULL){
		ESP_LOGE(UDP_SERVER_LOG_TAG,"UDP server already stoped");
		return ESP_FAIL;
	}
	vTaskDelete(udp_server_task_handle);
	close(udp_sct);
	udp_sct=-1;
	udp_server_task_handle=NULL;
	return ESP_OK;
}

