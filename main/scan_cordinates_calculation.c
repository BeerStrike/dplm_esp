#include "scan_cordinates_calculation.h"
#include "room_cords_math.h"
#include "esp_log.h"

esp_err_t calculate_next_point_at_floor_frw(struct scan_status *status);
esp_err_t calculate_next_point_at_x_wall_frw(struct scan_status *status);
esp_err_t calculate_next_point_at_y_wall_frw(struct scan_status *status);

esp_err_t calculate_next_point_at_floor_rev(struct scan_status *status);
esp_err_t calculate_next_point_at_x_wall_rev(struct scan_status *status);
esp_err_t calculate_next_point_at_y_wall_rev(struct scan_status *status);

esp_err_t initalize_cords_calculation(struct cords_limiter *room_cords_limiter,struct point *scaner_pos,enum direction scaner_direction,float scan_step, struct scan_status *status){
	status->scan_reverse=0;
	if(calculate_local_cords_limiter(room_cords_limiter,scaner_pos,scaner_direction,&status->limiter)!=ESP_OK){
		ESP_LOGE(SCAN_CORDS_LOG_TAG,"Error initalize cords calculation");
		return ESP_FAIL;
	}
	status->curr_point.x=0;
	status->curr_point.y=0;
	status->curr_point.h=status->limiter.h_lim;
	status->scan_step=scan_step;
	status->curr_plane=room_floor;
	status->dir_hor=frw;
	status->dir_vert=up;
	ESP_LOGI(SCAN_CORDS_LOG_TAG,"Cords calculation init sucsessful");
	return ESP_OK;
}

esp_err_t calculate_next_point_to_scan(struct scan_status *status){
	if(!status->scan_reverse){
		switch (status->curr_plane){
			case room_floor:
				calculate_next_point_at_floor_frw(status);
				break;
			case x_wall:
				calculate_next_point_at_x_wall_frw(status);
				break;
			case y_wall:
				calculate_next_point_at_y_wall_frw(status);
				break;
		}
	}
	ESP_LOGI(SCAN_CORDS_LOG_TAG,"Calculatd next local cord is x:%f, y:%f, h:%f",status->curr_point.x,status->curr_point.y,status->curr_point.h);
	return ESP_OK;
}

esp_err_t calculate_next_point_at_floor_frw(struct scan_status *status){
	if(status->dir_hor==frw)
		status->curr_point.y+=status->scan_step;
	else
		status->curr_point.y-=status->scan_step;
	if(status->curr_point.y<0){
		status->curr_point.y=0;
		status->dir_hor=frw;
		status->curr_point.x+=status->scan_step;
		if(status->curr_point.x>status->limiter.x_lim){
			status->curr_point.x=status->limiter.x_lim;
			status->curr_plane=y_wall;
			status->dir_vert=up;
			status->dir_hor=frw;
		}
	}
	if(status->curr_point.y>status->limiter.y_lim){
		status->curr_point.y=status->limiter.y_lim;
		status->curr_plane=x_wall;
		status->dir_vert=up;
	}
	return ESP_OK;
}

esp_err_t calculate_next_point_at_x_wall_frw(struct scan_status *status){
	if(status->dir_vert==up)
		status->curr_point.h-=status->scan_step;
	else
		status->curr_point.h+=status->scan_step;
	if(status->curr_point.h<0){
		status->curr_point.h=0;
		status->dir_vert=down;
		status->curr_point.x+=status->scan_step;
		if(status->curr_point.x>status->limiter.x_lim){
			status->curr_point.x=status->limiter.x_lim;
			status->curr_plane=y_wall;
			status->dir_vert=down;
			status->dir_hor=rev;
		}
	}
	if(status->curr_point.h>status->limiter.h_lim){
		status->curr_point.h=status->limiter.h_lim;
		status->curr_plane=room_floor;
		status->dir_hor=rev;
	}
	return ESP_OK;
}

esp_err_t calculate_next_point_at_y_wall_frw(struct scan_status *status){
	if(status->dir_hor==frw)
		status->curr_point.y+=status->scan_step;
	else
		status->curr_point.y-=status->scan_step;
	if(status->curr_point.y<0){
		status->curr_point.y=0;
		status->dir_hor=frw;
		if(status->dir_vert==up)
			status->curr_point.h-=status->scan_step;
		else
			status->curr_point.h+=status->scan_step;

	}
	if(status->curr_point.y>status->limiter.y_lim){
		status->curr_point.y=status->limiter.y_lim;
		status->dir_hor=rev;
		if(status->dir_vert==up)
			status->curr_point.h-=status->scan_step;
		else
			status->curr_point.h+=status->scan_step;
	}
	if(status->curr_point.h<0){
		status->curr_point.h=0;
		status->scan_reverse=1;
	}
	if(status->curr_point.h>status->limiter.h_lim){
		status->curr_point.h=status->limiter.h_lim;
		status->scan_reverse=1;
	}
	return ESP_OK;
}
