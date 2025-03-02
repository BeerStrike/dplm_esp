#ifndef LASER_RANGE_H_
#define LASER_RANGE_H_
#include "esp_err.h"
#define	LASER_RANGE_LOG_TAG "Laser range"
#define LASER_RANGE_ADDRESS 0b0101001

struct laser_range_SequenceStepEnables
{
	uint8_t tcc, msrc, dss, pre_range, final_range;
};
struct laser_range_SequenceStepTimeouts
{
	uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

	uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
	uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
};
enum laser_range_vcselPeriodType { VcselPeriodPreRange, VcselPeriodFinalRange };

uint8_t laser_range_stop_variable;
uint32_t laser_range_measurement_timing_budget_us;

void laser_range_init();
esp_err_t write_byte_to_laser_range(uint8_t address,uint8_t byte);
esp_err_t write_to_laser_range(uint8_t address,void *data,int l);
esp_err_t read_byte_from_laser_range(uint8_t address,uint8_t *byte);
esp_err_t read_from_laser_range(uint8_t address,void *data,int l);
void laser_range_getSpadInfo(uint8_t * count, uint8_t * type_is_aperture);
uint8_t laser_range_setSignalRateLimit(float limit_Mcps);
uint32_t laser_range_getMeasurementTimingBudget();
void laser_range_setMeasurementTimingBudget(uint32_t budget_us);
void laser_range_getSequenceStepEnables(struct laser_range_SequenceStepEnables * enables);
void laser_range_getSequenceStepTimeouts(struct laser_range_SequenceStepEnables const * enables,struct laser_range_SequenceStepTimeouts * timeouts);
uint8_t laser_range_getVcselPulsePeriod(enum laser_range_vcselPeriodType type);
uint32_t laser_range_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
uint32_t laser_range_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
uint16_t laser_range_decodeTimeout(uint16_t reg_val);
void laser_range_performSingleRefCalibration(uint8_t vhv_init_byte);
uint16_t laser_range_encodeTimeout(uint32_t timeout_mclks);
uint16_t laser_range_readRangeSingleMillimeters();
#endif /* LASER_RANGE_H_ */
