#include "room_cords_math.h"
#include "esp_log.h"
#include <math.h>

esp_err_t calculate_local_cords_limiter(struct cords_limiter *room_cords_limiter,struct point *scaner_pos,enum direction scaner_direction,struct cords_limiter *local_cords_limiter){
	local_cords_limiter->h_lim=scaner_pos->h;
	switch(scaner_direction){
			case xdyd:
				local_cords_limiter->x_lim=room_cords_limiter->x_lim-scaner_pos->x;
				local_cords_limiter->y_lim=room_cords_limiter->y_lim-scaner_pos->y;
				break;
			case xdyr:
				local_cords_limiter->x_lim=scaner_pos->y;
				local_cords_limiter->y_lim=room_cords_limiter->x_lim-scaner_pos->x;
				break;
			case xryd:
				local_cords_limiter->x_lim=room_cords_limiter->y_lim-scaner_pos->y;
				local_cords_limiter->y_lim=scaner_pos->x;
				break;
			case xryr:
				local_cords_limiter->x_lim=scaner_pos->x;
				local_cords_limiter->y_lim=scaner_pos->y;
				break;
		}
	return ESP_OK;
}

esp_err_t check_cords_limiter(struct point *verifable_point,struct cords_limiter *verifable_cords_limiter){
	if(verifable_point->x<0||verifable_point->x>verifable_cords_limiter->x_lim||
		verifable_point->y<0||verifable_point->y>verifable_cords_limiter->y_lim||
		verifable_point->h<0||verifable_point->h>verifable_cords_limiter->h_lim){
		ESP_LOGE(CORDS_MATH_LOG_TAG,"Error: cords x:%f, y:%f, h:%f out of cords limit",verifable_point->x,verifable_point->y,verifable_point->h);
		return ESP_FAIL;
	}
	return ESP_OK;
}


esp_err_t local_cords_to_global_cords(struct point *scaner_pos,enum direction scaner_direction,struct point *local_point,struct point *global_point){
	global_point->h=scaner_pos->h-local_point->h;
	switch(scaner_direction){
		case xdyd:
			global_point->x=scaner_pos->x+local_point->x;
			global_point->y=scaner_pos->y+local_point->y;
			break;
		case xdyr:
			global_point->x=scaner_pos->x+local_point->y;
			global_point->y=scaner_pos->y-local_point->x;
			break;
		case xryd:
			global_point->x=scaner_pos->x-local_point->y;
			global_point->y=scaner_pos->y+local_point->x;
			break;
		case xryr:
			global_point->x=scaner_pos->x-local_point->x;
			global_point->y=scaner_pos->y-local_point->y;
			break;
	}
	ESP_LOGI(CORDS_MATH_LOG_TAG,"Global point x:%f, y:%f, h:%f",global_point->x,global_point->y,global_point->h);
	return ESP_OK;
}

float calculate_yaw(struct point *local_point){
	return atan(sqrt(local_point->x*local_point->x+local_point->y*local_point->y)/local_point->h)/ M_PI * 180;
}

float calculate_pitch(struct point *local_point){
	return atan((local_point->y) / (local_point->x)) / M_PI * 180;
}

float calculate_distanse_to_point(struct point *local_point){
	return sqrt(local_point->x*local_point->x+local_point->y*local_point->y+local_point->h*local_point->h);
}

esp_err_t calculate_real_point(struct point *scaning_point,float range,struct point *real_point){
	ESP_LOGI(CORDS_MATH_LOG_TAG,"Scaning local point x:%f, y:%f, h:%f",scaning_point->x,scaning_point->y,scaning_point->h);
	float mp=range/calculate_distanse_to_point(scaning_point);
	real_point->x=scaning_point->x*mp;
	real_point->y=scaning_point->y*mp;
	real_point->h=scaning_point->h*mp;
	ESP_LOGI(CORDS_MATH_LOG_TAG,"Real local point x:%f, y:%f, h:%f",real_point->x,real_point->y,real_point->h);
	return ESP_OK;
}


