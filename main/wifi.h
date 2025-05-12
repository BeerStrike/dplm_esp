#ifndef WIFI_FUNCTIONS_H_
#define WIFI_FUNCTIONS_H_
#include "esp_err.h"
#define WIFI_LOG_TAG "Wifi"

esp_err_t wifi_init();
esp_err_t wifi_start(char *wifi_ssid,char *wifi_pass);
esp_err_t wifi_stop();

#endif /* WIFI_FUNCTIONS_H_ */
