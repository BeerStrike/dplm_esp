#ifndef ROOM_CORDS_MATH_H_
#define ROOM_CORDS_MATH_H_
#include "esp_err.h"
#define CORDS_MATH_LOG_TAG "Cords math"

enum direction {xdyd,xryd,xdyr,xryr};

struct point{
	float x;
	float y;
	float h;
};

struct cords_limiter{
	float x_lim;
	float y_lim;
	float h_lim;
};


struct scan_parameters {
	struct cords_limiter room_cords_limiter;
	float scan_step;
	struct point scaner_pos;
	enum direction scaner_direction;
};

esp_err_t calculate_local_cords_limiter(struct scan_parameters *params,struct cords_limiter *local_cords_limiter);
esp_err_t check_cords_limiter(struct point *verifable_point,struct cords_limiter *verifable_cords_limiter);
esp_err_t global_cords_to_local_cords(struct scan_parameters *params,struct point *global_point,struct point *local_point);
esp_err_t local_cords_to_global_cords(struct scan_parameters *params,struct point *local_point,struct point *global_point);
float calculate_yaw(struct point *local_point);
float calculate_pitch(struct point *local_point);
float calculate_distanse_to_point(struct point *local_point);
esp_err_t calculate_real_point(struct point *scaning_point,float range,struct point *real_point);

#endif /* ROOM_CORDS_MATH_H_ */
