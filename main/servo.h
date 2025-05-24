#ifndef SERVO_H_
#define SERVO_H_
#include "esp_err.h"

#define	SERVO_LOG_TAG "Servo"
#define MIN_PWM 550
#define MAX_PWM 1600

esp_err_t servo_init();
esp_err_t set_yaw(float yaw);
esp_err_t set_pitch(float pitch);
esp_err_t servo_stop();
#endif
