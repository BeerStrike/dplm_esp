#include "udp_server.h"
#include "lwip/sockets.h"
#include "esp_log.h"

int udp_server_init(){
	int sct;
	struct sockaddr_in  listening_addr;
	memset(&listening_addr, 0, sizeof(listening_addr));
	listening_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listening_addr.sin_family = AF_INET;
	listening_addr.sin_port = htons(UDP_SERVER_PORT);
	sct=socket(AF_INET,SOCK_DGRAM ,IPPROTO_UDP);
	if(sct<0){
		ESP_LOGI(UDP_SERVER_LOG_TAG,"Error create UDP socket");
		return -1;
	}
	if(bind(sct,(struct sockaddr *)&listening_addr,sizeof(listening_addr))<0){
		close(sct);
		ESP_LOGE(UDP_SERVER_LOG_TAG,"Error bind UDP socket");
		return -1;
	}
	return sct;
}

void udp_server_task(void *params){
    ESP_LOGI(UDP_SERVER_LOG_TAG,"UDP server started");
    char rx_buffer[UDP_SERVER_RX_BUFFER_SIZE];
    int sct;
    do{
    	sct=udp_server_init();
    }while(sct==-1);
    ESP_LOGI(UDP_SERVER_LOG_TAG,"Start listening UDP");
	while(true){
		struct sockaddr_in sourceAddr;
        socklen_t socklen = sizeof(sourceAddr);
		int len =recvfrom(sct, rx_buffer, sizeof(rx_buffer), 0, (struct sockaddr *)&sourceAddr, &socklen);
		if (len < 0) {
			ESP_LOGI(UDP_SERVER_LOG_TAG,"UDP recive error");
		}else{
			ESP_LOGI(UDP_SERVER_LOG_TAG,"Recive UDP");
			char json[]="{\t\"type\": \"Find_response\"}";
            if(sendto(sct, json, strlen(json), 0, (struct sockaddr *)&sourceAddr, sizeof(sourceAddr))<0)
    			ESP_LOGE(UDP_SERVER_LOG_TAG,"UDP send error");
            else
    			ESP_LOGI(UDP_SERVER_LOG_TAG,"UDP send");
		}
	}
}


