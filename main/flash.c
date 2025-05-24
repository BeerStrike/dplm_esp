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
	size_t len = 32;
	err=nvs_get_str(nvs, "Wi-Fi SSID", wifi_ssid,&len);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error read ssid: %d",err);
		return err;
	}
    len = 64;
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
	ESP_LOGI(FLASH_LOG_TAG, "Wifi settings saved to flash");
	return ESP_OK;
}

esp_err_t load_scaner_name_from_flash(char *scaner_name){
	nvs_handle nvs;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
	    return err;
	}
	size_t len = 64;
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
	ESP_LOGI(FLASH_LOG_TAG, "Scaner name saved to flash");
	return ESP_OK;
}

esp_err_t load_port_from_flash(int16_t *port){
	nvs_handle nvs;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
	    return err;
	}
	nvs_get_i16(nvs, "Port", port);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error read port: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Port loaded from flash");
	return ESP_OK;
}

esp_err_t save_port_to_flash(int16_t port){
	nvs_handle nvs;
	esp_err_t err;
	err = nvs_open("settings", NVS_READWRITE, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
		return err;
	}
	nvs_set_i16(nvs, "Port", port);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error write port: %d",err);
		return err;
	}
	err=nvs_commit(nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash commit: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG,"Port saved to flash");
	return ESP_OK;
}

esp_err_t load_range_calibration_from_flash(int16_t *range_calibration){
	nvs_handle nvs;
	esp_err_t err = nvs_open("settings", NVS_READONLY, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
	    return err;
	}
	nvs_get_i16(nvs, "Range calib", range_calibration);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error read port: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG, "Range calibration loaded from flash");
	return ESP_OK;
}

esp_err_t save_range_calibration_to_flash(int16_t range_calibration){
	nvs_handle nvs;
	esp_err_t err;
	err = nvs_open("settings", NVS_READWRITE, &nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash open: %d",err);
		return err;
	}
	nvs_set_i16(nvs, "Range calib", range_calibration);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error write port: %d",err);
		return err;
	}
	err=nvs_commit(nvs);
	if (err != ESP_OK){
		ESP_LOGE(FLASH_LOG_TAG, "Error flash commit: %d",err);
		return err;
	}
	nvs_close(nvs);
	ESP_LOGI(FLASH_LOG_TAG,"Range calibration saved to flash");
	return ESP_OK;
}
