#include "flash.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"


esp_err_t load_wifi_settings_from_flash(char *wifi_ssid,char *wifi_pass){
	nvs_handle nvs;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
	    return err;
	}
	size_t len = sizeof(wifi_ssid);
	err=nvs_get_str(nvs, "Wi-Fi SSID", wifi_ssid,&len);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error read ssid: %d",err);
		return err;
	}
    len = sizeof(wifi_pass);
    err=nvs_get_str(nvs, "Wi-Fi password", wifi_pass,&len);
	if (err != ESP_OK)	{
		ESP_LOGE(FLASH_LOG_TAG, "Error read password: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Wifi Settings loaded from flash");
	return ESP_OK;
}

esp_err_t save_wifi_settings_to_flash(char *wifi_ssid,char *wifi_pass){
	nvs_handle nvs;
	esp_err_t err;
	err = nvs_open("settings", NVS_READWRITE, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
		return err;
	}
	err=nvs_set_str(nvs, "Wi-Fi SSID", wifi_ssid);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error write ssid: %d",err);
		return err;
	}
	err=nvs_set_str(nvs, "Wi-Fi password", wifi_pass);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error write password: %d",err);
		return err;
	}
	err=nvs_commit(nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash commit: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Settings saved to flash");
	return ESP_OK;
}

esp_err_t load_scaner_name_from_flash(char *scaner_name){
	nvs_handle nvs;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
	    return err;
	}
	size_t len = sizeof(scaner_name);
	err=nvs_get_str(nvs, "Scaner name", scaner_name,&len);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error read name: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Scaner name loaded from flash");
	return ESP_OK;
}

esp_err_t save_scaner_name_to_flash(char *scaner_name){
	nvs_handle nvs;
	esp_err_t err;
	err = nvs_open("settings", NVS_READWRITE, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
		return err;
	}
	err=nvs_set_str(nvs, "Scaner name", scaner_name);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error write name: %d",err);
		return err;
	}
	err=nvs_commit(nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash commit: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Scaner name loaded from flash");
	return ESP_OK;
}

