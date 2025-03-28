#ifndef SERVO_H_
#define SERVO_H_
#define	SERVO_LOG_TAG "Servo"

void servo_init();
void set_yaw(float yaw);
void set_pitch(float pitch);
void servo_stop();
#endif
