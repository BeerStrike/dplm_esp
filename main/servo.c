#include "servo.h"
#include "driver/pwm.h"
#include "esp_err.h"
#include "esp_log.h"

esp_err_t servo_init(){
	const uint32_t pin_num[2] = {
	    12,
	    14,
	};
	uint32_t duties[2] = {
			0, 0
	};
	float phase[2] = {
	    0, 0
	};
	esp_err_t err=pwm_init(20000,duties,2,pin_num);
	if(err!=ESP_OK){
		ESP_LOGE(SERVO_LOG_TAG,"Error pwm init: %d",err);
		return err;
	}
	err=pwm_set_phases(phase);
	 if(err!=ESP_OK){
	    ESP_LOGE(SERVO_LOG_TAG,"Error set phase: %d",err);
		return err;
	 }
    err=pwm_start();
    if(err!=ESP_OK){
    	ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);
		return err;
    }
	ESP_LOGI(SERVO_LOG_TAG,"Servo initalized");
    return ESP_OK;
}

esp_err_t set_yaw(float yaw){
	if(yaw<0||yaw>90.0){
		ESP_LOGE(SERVO_LOG_TAG,"Error invalid yaw: %f",yaw);
		return ESP_FAIL;
	}
	int duty=yaw/90.0f*(MAX_PWM-MIN_PWM)+MIN_PWM;
	pwm_set_duty(0,duty);
	 esp_err_t err=pwm_start();
	 if(err!=ESP_OK){
	 	ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);
	 	return err;
	 }
	ESP_LOGI(SERVO_LOG_TAG,"Servo set yaw:%f",yaw);
	return ESP_OK;
}

esp_err_t set_pitch(float pitch){
	if(pitch<0||pitch>90.0){
		ESP_LOGE(SERVO_LOG_TAG,"Error invalid pitch: %f",pitch);
		return ESP_FAIL;
	}
	int duty=(90.0f-pitch)/90.0f*(MAX_PWM-MIN_PWM)+MIN_PWM;
	pwm_set_duty(1,duty);
	esp_err_t err=pwm_start();
	if(err!=ESP_OK){
		ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);
		return err;
	}
	ESP_LOGI(SERVO_LOG_TAG,"Servo set pitch:%f",pitch);
	return ESP_OK;
}

esp_err_t servo_stop(){
	 esp_err_t err=pwm_stop(0);
	 if(err!=ESP_OK){
	 		ESP_LOGE(SERVO_LOG_TAG,"Error pwm stop: %d",err);
	 		return err;
	 }
	ESP_LOGI(SERVO_LOG_TAG,"Servo stoped");
	 return ESP_OK;
}

