#ifndef SCAN_H_
#define SCAN_H_
#define	SCAN_LOG_TAG "Scaning"
#include "room_cords_math.h"
#define SCANER_LENGTH 0.057f
#define SCANER_WIDTH 0.023f
#define SCANER_HEIGHT 0.034f

struct scan_parameters {
	struct cords_limiter room_cords_limiter;
	float scan_step;
	struct point scaner_pos;
	enum direction scaner_direction;
};
enum scan_state{not_started,working,paused};

esp_err_t start_scan(struct scan_parameters params);
esp_err_t pause_scan();
esp_err_t continue_scan();
esp_err_t stop_scan();
enum scan_state get_scan_state();



#endif /* SCAN_H_ */
