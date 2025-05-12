#ifndef MAIN_FLASH_H_
#define MAIN_FLASH_H_
#include "esp_err.h"
#define FLASH_LOG_TAG "Flash"

esp_err_t load_wifi_settings_from_flash(char *wifi_ssid,char *wifi_pass);
esp_err_t save_wifi_settings_to_flash(char *wifi_ssid,char *wifi_pass);
esp_err_t load_scaner_name_from_flash(char *scaner_name);
esp_err_t save_scaner_name_to_flash(char *scaner_name);

#endif /* MAIN_FLASH_H_ */
