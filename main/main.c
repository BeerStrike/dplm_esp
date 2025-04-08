#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "wifi_functions.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "globals.h"
#include "laser_range.h"
#include "servo.h"
#include "uart.h"
#include <string.h>
#define	MAIN_LOG_TAG "Main"


void app_main()
{
    tcpip_adapter_init();
    esp_event_loop_create_default();
    UART_init();
	strcpy(wifi_ssid,"Kal");
	strcpy(wifi_pass,"07111917");
	strcpy(scaner_name,"Lenin");
    i2c_init();
    laser_range_init(1);
    laser_range_setTimeout(1000);
    servo_init();
	//wifi_init(wifi_ssid,wifi_pass);
	//start_udp_server();
	//start_tcp_server();
    vTaskDelete(NULL);
}

