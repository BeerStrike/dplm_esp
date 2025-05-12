#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "laser_range.h"
#include "servo.h"
#include "uart.h"
#include "flash.h"
#include "wifi.h"

void app_main()
{
	if(uart_init()==ESP_OK){
		if(i2c_init()==ESP_OK){
			if(servo_init()==ESP_OK){
				if(wifi_init()==ESP_OK){
					char wifi_ssid[32];
					char wifi_pass[64];
					if(load_wifi_settings_from_flash(wifi_ssid,wifi_pass)==ESP_OK){
						wifi_start(wifi_ssid,wifi_pass);
					}
				}
			}
		}
	}
    vTaskDelete(NULL);
}

