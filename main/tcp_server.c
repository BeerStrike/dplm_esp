#include "tcp_server.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "json_functions.h"

int tcp_server_init(){
	ESP_LOGI(TCP_SERVER_LOG_TAG,"Tcp server started");
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
			  tcp_sct=-1;
			  break;
		 }else{
			 ESP_LOGI(TCP_SERVER_LOG_TAG,"Recive TCP");
			 rx_buffer[len]='\0';
			 printf(rx_buffer);
			 json_recive_processor(rx_buffer);
		 }
	}
}
void tcp_sender_task(void * params){

}

void tcp_server_task(void *params){
    int sct;
    do{
    	sct=tcp_server_init();
    }while(sct==-1);
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
