#ifndef WIFI_FUNCTIONS_H_
#define WIFI_FUNCTIONS_H_
#include "esp_wifi.h"
#define WIFI_LOG_TAG "Wifi"

void wifi_init();
void wifi_event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data);

#endif /* WIFI_FUNCTIONS_H_ */
