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
#include "flash.h"

void app_main()
{
    tcpip_adapter_init();
    esp_event_loop_create_default();
    UART_init();
    i2c_init();
    laser_range_init(1);
    laser_range_setTimeout(1000);
    servo_init();
    load_settings_from_flash();
	wifi_init();
	start_udp_server();
	start_tcp_server();
    vTaskDelete(NULL);
}

