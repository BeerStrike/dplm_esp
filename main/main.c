#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "wifi_functions.h"
#include "udp_server.h"
#include "tcp_server.h"
#include "globals.h"
#include "laser_range.h"
#include "servo.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#define	MAIN_LOG_TAG "Main"


void app_main()
{
    tcpip_adapter_init();
    esp_event_loop_create_default();
    i2c_init();
    laser_range_init(1);
    servo_init();
	wifi_init(WIFI_SSID,WIFI_PASS);
	start_udp_server();
	start_tcp_server();
    vTaskDelete(NULL);
}

