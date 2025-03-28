#include "servo.h"
#include "driver/pwm.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void servo_init(){
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
	if(err!=ESP_OK)
		ESP_LOGE(SERVO_LOG_TAG,"Error pwm inst: %d",err);
	err=pwm_set_phases(phase);
	 if(err!=ESP_OK)
	    ESP_LOGE(SERVO_LOG_TAG,"Error set phase: %d",err);
    err=pwm_start();
    if(err!=ESP_OK)
    	ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);
}

void set_yaw(float yaw){
	int duty=yaw/90.0f*1000+500;
	pwm_set_duty(0,duty);
	 esp_err_t err=pwm_start();
	 if(err!=ESP_OK)
	 	ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);

}

void set_pitch(float pitch){
	int duty=pitch/90.0f*1000+500;
	pwm_set_duty(1,duty);
	esp_err_t err=pwm_start();
	if(err!=ESP_OK)
		ESP_LOGE(SERVO_LOG_TAG,"Error pwm start: %d",err);
}

void servo_stop(){
	 esp_err_t err=pwm_stop(0);
	 if(err!=ESP_OK)
	 		ESP_LOGE(SERVO_LOG_TAG,"Error pwm stop: %d",err);
}

