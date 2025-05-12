#include "scan_cordinates_calculation.h"
#include "room_cords_math.h"
#include "esp_log.h"

esp_err_t initalize_cords_calculation(struct scan_parameters *params, struct scan_status *status){
	status->x_reverse=0;
	status->y_reverse=0;
	if(calculate_local_cords_limiter(params,&status->limiter)!=ESP_OK){
		ESP_LOGE(SCAN_CORDS_LOG_TAG,"Error initalize cords calculation");
		return ESP_FAIL;
	}
	status->curr_point.x=0;
	status->curr_point.y=0;
	status->curr_point.h=status->limiter.h_lim;
	status->scan_step=params->scan_step;
	status->curr_plane=room_floor;
	ESP_LOGI(SCAN_CORDS_LOG_TAG,"Cords calculation init sucsessful");
	return ESP_OK;
}

esp_err_t calculate_next_point_to_scan(struct scan_status *status){
	switch (status->curr_plane){
		case room_floor:
			calculate_next_point_at_floor(status);
			break;
		case x_wall:
			calculate_next_point_at_x_wall(status);
			break;
		case y_wall:
			calculate_next_point_at_y_wall(status);
			break;
	}

	if(status->curr_point.h>status->limiter.h_lim){
		status->y_reverse=0;
		status->curr_point.h=status->limiter.h_lim;
	}
	ESP_LOGI(SCAN_CORDS_LOG_TAG,"Calculatd next local cord is x:%f, y:%f, h:%f",status->curr_point.x,status->curr_point.y,status->curr_point.h);
	return ESP_OK;
}

esp_err_t calculate_next_point_at_floor(struct scan_status *status){
	if(!status->y_reverse){
		status->curr_point.y+=status->scan_step;
	}else{
		status->curr_point.y-=status->scan_step;
	}
	if(status->curr_point.y<0){
		status->curr_point.y=0;
		status->y_reverse=0;
		status->curr_point.x+=status->scan_step;
	}
	if(status->curr_point.y>status->limiter.y_lim){
		status->curr_point.y=status->limiter.y_lim;
		status->curr_plane=x_wall;
	}
	return ESP_OK;
}

esp_err_t calculate_next_point_at_x_wall(struct scan_status *status){
	if(!status->y_reverse){
		status->curr_point.h-=status->scan_step;
	}else{
		status->curr_point.h+=status->scan_step;
	}
	if(status->curr_point.h>status->limiter.h_lim){
			status->curr_point.h=status->limiter.h_lim;
			status->curr_plane=room_floor;
	}
	if(status->curr_point.h<0){
		status->curr_point.h=0;
		status->y_reverse=1;
		status->curr_point.x+=status->scan_step;
	}
	return ESP_OK;
}

esp_err_t calculate_next_point_at_y_wall(struct scan_status *status){
	return ESP_OK;
}
