#ifndef SCAN_CORDINATES_CALCULATION_H_
#define SCAN_CORDINATES_CALCULATION_H_
#include "esp_err.h"
#include "room_cords_math.h"
#define SCAN_CORDS_LOG_TAG "Scan cords calculation"

enum plane {room_floor,x_wall,y_wall};

struct scan_status{
	int x_reverse;
	int y_reverse;
	float scan_step;
	struct point curr_point;
	struct cords_limiter limiter;
	enum plane curr_plane;
};

esp_err_t initalize_cords_calculation(struct scan_parameters *params, struct scan_status *status);
esp_err_t calculate_next_point_to_scan(struct scan_status *status);
esp_err_t calculate_next_point_at_floor(struct scan_status *status);
esp_err_t calculate_next_point_at_x_wall(struct scan_status *status);
esp_err_t calculate_next_point_at_y_wall(struct scan_status *status);

#endif /* SCAN_CORDINATES_CALCULATION_H_ */
