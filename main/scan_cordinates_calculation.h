#ifndef SCAN_CORDINATES_CALCULATION_H_
#define SCAN_CORDINATES_CALCULATION_H_
#include "esp_err.h"
#include "room_cords_math.h"
#define SCAN_CORDS_LOG_TAG "Scan cords calculation"

enum plane {room_floor,x_wall,y_wall};
enum direction_hor{frw,rev};
enum direction_vert{up,down};

struct scan_status{
	int scan_reverse;
	float scan_step;
	struct point curr_point;
	struct cords_limiter limiter;
	enum plane curr_plane;
	enum direction_hor dir_hor;
	enum direction_vert dir_vert;
};

esp_err_t initalize_cords_calculation(struct cords_limiter *room_cords_limiter,struct point *scaner_pos,enum direction scaner_direction,float scan_step, struct scan_status *status);
esp_err_t calculate_next_point_to_scan(struct scan_status *status);

#endif /* SCAN_CORDINATES_CALCULATION_H_ */
