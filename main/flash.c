#include "flash.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "globals.h"

void load_settings_from_flash(){
    nvs_handle nvs;
    esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
    if (err != ESP_OK) ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
    size_t len = sizeof(wifi_ssid);
	err=nvs_get_str(nvs, "Wi-Fi SSID", wifi_ssid,&len);
	if (err != ESP_OK)	ESP_LOGE(FLASH_LOG_TAG, "Error read ssid: %d",err);

    len = sizeof(wifi_pass);
	err=nvs_get_str(nvs, "Wi-Fi password", wifi_pass,&len);
	if (err != ESP_OK)	ESP_LOGE(FLASH_LOG_TAG, "Error read password: %d",err);

    len = sizeof(scaner_name);
	err=nvs_get_str(nvs, "Scaner name", scaner_name,&len);
	if (err != ESP_OK)	ESP_LOGE(FLASH_LOG_TAG, "Error read name: %d",err);

	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Settings loaded from flash");
}

void save_settings_to_flash(){
	nvs_handle nvs;
	esp_err_t err;
	err = nvs_open("settings", NVS_READWRITE, &nvs);
	if (err != ESP_OK) ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);

	err=nvs_set_str(nvs, "Wi-Fi SSID", wifi_ssid);
	if (err != ESP_OK) 	ESP_LOGE(FLASH_LOG_TAG, "Error write ssid: %d",err);

	err=nvs_set_str(nvs, "Wi-Fi password", wifi_pass);
	if (err != ESP_OK) 	ESP_LOGE(FLASH_LOG_TAG, "Error write password: %d",err);

	err=nvs_set_str(nvs, "Scaner name", scaner_name);
	if (err != ESP_OK) 	ESP_LOGE(FLASH_LOG_TAG, "Error write name: %d",err);

	err=nvs_commit(nvs);
	if (err != ESP_OK) ESP_LOGE(FLASH_LOG_TAG, "Error flash commit: %d",err);

	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Settings saved to flash");
}
