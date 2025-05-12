#include "tcp_server.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "cJSON.h"
#include "room_cords_math.h"
#include "scan.h"
int tcp_sct=0;
TaskHandle_t tcp_server_task_handle=NULL;
char scaner_name[64];
void scan_params_json_handler(cJSON * json);
void state_request_json_handler(cJSON *json);
void json_recive_processor(char *jsonstr);


void scan_params_json_handler(cJSON * json){
	struct scan_parameters params;
	cJSON *length = cJSON_GetObjectItemCaseSensitive(json, "Room length");
	cJSON *width = cJSON_GetObjectItemCaseSensitive(json, "Room width");
	cJSON *height = cJSON_GetObjectItemCaseSensitive(json, "Room height");
	cJSON *step = cJSON_GetObjectItemCaseSensitive(json, "Scan step");
	cJSON *x = cJSON_GetObjectItemCaseSensitive(json, "Scaner X");
	cJSON *y = cJSON_GetObjectItemCaseSensitive(json, "Scaner Y");
	cJSON *h = cJSON_GetObjectItemCaseSensitive(json, "Scaner H");
	cJSON *dr = cJSON_GetObjectItemCaseSensitive(json, "Scaner direction");
	if(length!=NULL&&width!=NULL&&height!=NULL&&step!=NULL
			&&cJSON_IsNumber(length)&&cJSON_IsNumber(width)&&cJSON_IsNumber(height)&&cJSON_IsNumber(step)){
		params.room_cords_limiter.x_lim=length->valuedouble;
		params.room_cords_limiter.y_lim=width->valuedouble;
		params.room_cords_limiter.h_lim=height->valuedouble;
		params.scan_step=step->valuedouble;
		params.scaner_pos.h=h->valuedouble-SCANER_HEIGHT;
		if(strcmp(dr->valuestring,"xdyd")==0){
			params.scaner_pos.x=x->valuedouble+SCANER_WIDTH;
			params.scaner_pos.y=y->valuedouble+SCANER_LENGTH;
			params.scaner_direction=xdyd;
		}else if(strcmp(dr->valuestring,"xdyr")==0){
			params.scaner_pos.x=x->valuedouble-SCANER_LENGTH;
			params.scaner_pos.y=y->valuedouble+SCANER_WIDTH;
			params.scaner_direction=xdyr;
		}else if(strcmp(dr->valuestring,"xryd")==0){
			params.scaner_pos.x=x->valuedouble-SCANER_LENGTH;
			params.scaner_pos.y=y->valuedouble+SCANER_WIDTH;
			params.scaner_direction=xryd;
		}else if(strcmp(dr->valuestring,"xryr")==0){
			params.scaner_pos.x=x->valuedouble-SCANER_WIDTH;
			params.scaner_pos.y=y->valuedouble-SCANER_LENGTH;
			params.scaner_direction=xryr;
		}
		start_scan(params);
	}
}

void state_request_json_handler(cJSON *json){
	cJSON *response_json=cJSON_CreateObject();
	cJSON_AddItemToObject(response_json, "Type", cJSON_CreateString("State response"));
	if(is_scan_active())
		cJSON_AddItemToObject(response_json, "State", cJSON_CreateString("Working"));
	else
		cJSON_AddItemToObject(response_json, "State", cJSON_CreateString("Wait for params"));
	cJSON_AddItemToObject(response_json, "Scaner name", cJSON_CreateString(scaner_name));
	char *jsonstr=cJSON_Print(response_json);
	printf(jsonstr);
	send_to_pc(jsonstr);
	cJSON_free(json);
	cJSON_Delete(response_json);

}
void json_recive_processor(char *jsonstr){
	cJSON *json = cJSON_Parse(jsonstr);
	if(json!=NULL){
		cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
		if(type!=NULL&&type->valuestring!=NULL){
			if(strcmp(type->valuestring,"Scan parameters")==0)
				scan_params_json_handler(json);
			else if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"State request")==0)
				state_request_json_handler(json);
		}
		cJSON_Delete(json);
	}
}



esp_err_t start_tcp_server(){
	if(!tcp_server_task_handle){
		if(xTaskCreate(tcp_server_task,"Tcp server task",4096,&tcp_server_task_handle,1,NULL)!=pdTRUE){
			ESP_LOGE(TCP_SERVER_LOG_TAG,"Tcp server start error");
			return ESP_FAIL;
		}
	}else{
		ESP_LOGE(TCP_SERVER_LOG_TAG,"TCP server already started");
		return ESP_FAIL;
	}
	return ESP_OK;
}

esp_err_t stop_tcp_server(){
	if(tcp_server_task_handle){
		vTaskDelete(tcp_server_task_handle);
		tcp_server_task_handle=NULL;
		ESP_LOGI(TCP_SERVER_LOG_TAG,"TCP server stopped");
		return ESP_OK;
	}else{
		ESP_LOGE(TCP_SERVER_LOG_TAG,"TCP server already stopped");
		return ESP_FAIL;
	}
}

esp_err_t send_to_pc(char * payload){
	if(tcp_sct){
		if (send(tcp_sct,payload, strlen(payload), 0) < 0) {
			ESP_LOGE(TCP_SERVER_LOG_TAG,"Error send tcp");
			return ESP_FAIL;
		}else{
			ESP_LOGI(TCP_SERVER_LOG_TAG,"Send tcp: %s",payload);
			return ESP_OK;
		}
	}else{
		ESP_LOGE(TCP_SERVER_LOG_TAG,"PC not connected");
		return ESP_FAIL;
	}
}

int tcp_server_init(){
	ESP_LOGI(TCP_SERVER_LOG_TAG,"TCP server started");
	int sct;
	struct sockaddr_in  listening_addr;
    memset(&listening_addr, 0, sizeof(listening_addr));
	listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listening_addr.sin_family = AF_INET;
	listening_addr.sin_port = htons(TCP_SERVER_PORT);

	sct=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if(sct<0){
		ESP_LOGE(TCP_SERVER_LOG_TAG,"Error create TCP socket");
		return -1;
	}
	if(bind(sct,(struct sockaddr *)&listening_addr,sizeof(listening_addr))<0){
		close(sct);
		ESP_LOGE(TCP_SERVER_LOG_TAG,"Error bind TCP socket");
		return -1;
	}
	if(listen(sct,1)<0){
		close(sct);
		ESP_LOGE(TCP_SERVER_LOG_TAG,"Error start listen TCP socket");
		return -1;
	}
	ESP_LOGI(TCP_SERVER_LOG_TAG,"Start listening TCP");
	return sct;
}

void tcp_request_handle_cycle(int sct){
    char rx_buffer[TCP_SERVER_RX_BUFFER_SIZE];
	while (true) {
		 int len = recv(sct, rx_buffer, sizeof(rx_buffer) - 1, 0);
		 if (len < 0) {
			  ESP_LOGE(TCP_SERVER_LOG_TAG,"TCP recive error");
			  break;
		 }else if (len == 0) {
			  ESP_LOGI(TCP_SERVER_LOG_TAG,"TCP close connection");
			  tcp_sct=0;
			  break;
		 }else{
			 ESP_LOGI(TCP_SERVER_LOG_TAG,"Recive TCP");
			 rx_buffer[len]='\0';
			 ESP_LOGI(TCP_SERVER_LOG_TAG,"Recive TCP: %s",rx_buffer);
			 json_recive_processor(rx_buffer);
		 }
	}
}

void tcp_server_task(void *params){
    int sct=sct=tcp_server_init();
    if(sct==-1){
		ESP_LOGE(TCP_SERVER_LOG_TAG,"Error create tcp socket");
		vTaskDelete(NULL);
    }
	while(true){
		struct sockaddr_in sourceAddr;
		socklen_t socklen = sizeof(sourceAddr);
		int sctaccept=accept(sct, (struct sockaddr *)&sourceAddr, &socklen);
		if(sctaccept<0)
			ESP_LOGE(TCP_SERVER_LOG_TAG,"TCP accept error");
		else{
			ESP_LOGI(TCP_SERVER_LOG_TAG,"New TCP connection");
			tcp_sct=sctaccept;
			tcp_request_handle_cycle(sctaccept);
		}
	}
}



