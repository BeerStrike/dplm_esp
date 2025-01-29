#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#include "cJSON.h"

#define WIFI_SSID      ":-)"
#define WIFI_PASS      "22081609"
#define MYPORT 1917

struct udp_pcb *upcb;
struct tcp_pcb *tpcb;


static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);
void wifi_init();
void udp_recive_init();
void tcp_recive_init();
void udp_recive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
err_t tcp_recive_callback(void *arg, struct tcp_pcb *tpcb,struct pbuf *p, err_t err);
err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
void json_handler_task(void * params);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED){
    	udp_recive_init();
    	tcp_recive_init();
    }
}

void wifi_init()
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    if (strlen((char *)wifi_config.sta.password)) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
}

void udp_recive_init(){
	upcb = udp_new();
	udp_bind(upcb, IP_ADDR_ANY, MYPORT);
	udp_recv(upcb, udp_recive_callback, NULL);
}
void udp_recive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port){
	char json[]="{\n\t\"type\": \"Find_response\"\n}";
	struct pbuf *b=pbuf_alloc(PBUF_TRANSPORT,strlen(json),PBUF_RAM);
	memcpy(b->payload,json,strlen(json));
	udp_sendto(upcb,b,addr,port);
	pbuf_free(b);
}
void tcp_recive_init(){
	tpcb=tcp_new();
	tcp_bind(tpcb,IP_ADDR_ANY,MYPORT);
	tpcb=tcp_listen(tpcb);
	tcp_accept(tpcb,tcp_accept_callback);
}
err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err){
	tcp_recv(newpcb,tcp_recive_callback);
	printf("New tcp connection\n");
	//TODO вопрос памяти
	return ERR_OK;
}
err_t tcp_recive_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){
	if(tpcb==NULL){
		printf("Close tcp connection\n");
		return 1;
	}
	cJSON *json = cJSON_Parse(p->payload);
	if(json!=NULL){
		cJSON *type = cJSON_GetObjectItemCaseSensitive(json, "Type");
		if(type!=NULL&&type->valuestring!=NULL&&strcmp(type->valuestring,"Angles_to_scan")==0){
			cJSON *points = cJSON_GetObjectItemCaseSensitive(json, "Angles_array");
			const cJSON *point=NULL;
			printf("Angles array:\n");
			cJSON_ArrayForEach(point, points){
				cJSON *Yaw = cJSON_GetObjectItemCaseSensitive(point, "Yaw");
				cJSON *Pitch = cJSON_GetObjectItemCaseSensitive(point, "Pitch");
				if(Yaw!=NULL&&Pitch!=NULL&&cJSON_IsNumber(Yaw)&&cJSON_IsNumber(Pitch)){
					int y=(int)Yaw->valuedouble;
					int p=(int)Pitch->valuedouble;
					printf("%d || %d\n",y,p);
				}
			}
			cJSON_Delete(points);
		}
		cJSON_Delete(type);
	}
	cJSON_Delete(json);
	tcp_recved(tpcb,p->tot_len);
	return 1;
}

void app_main()
{
	wifi_init();
}
