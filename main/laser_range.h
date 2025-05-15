#ifndef LASER_RANGE_H_
#define LASER_RANGE_H_
#include "esp_err.h"
#define LASER_RANGE_LOG_TAG "Laser range"

struct laser_range_SequenceStepEnables{
  uint8_t tcc, msrc, dss, pre_range, final_range;
};

struct laser_range_SequenceStepTimeouts{
  	uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;
    uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
    uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
};
enum laser_range_vcselPeriodType { VcselPeriodPreRange, VcselPeriodFinalRange };

esp_err_t laser_range_get_last_status();
esp_err_t i2c_init();

esp_err_t laser_range_init(uint8_t io_2v8);

void laser_range_writeReg(uint8_t reg, uint8_t value);
void laser_range_writeReg16Bit(uint8_t reg, uint16_t value);
void laser_range_writeReg32Bit(uint8_t reg, uint32_t value);
uint8_t laser_range_readReg(uint8_t reg);
uint16_t laser_range_readReg16Bit(uint8_t reg);
uint32_t laser_range_readReg32Bit(uint8_t reg);

void laser_range_writeMulti(uint8_t reg, uint8_t  * src, uint8_t count);
void laser_range_readMulti(uint8_t reg, uint8_t * dst, uint8_t count);

esp_err_t laser_range_setSignalRateLimit(float limit_Mcps);
float laser_range_getSignalRateLimit();

esp_err_t laser_range_setMeasurementTimingBudget(uint32_t budget_us);
uint32_t laser_range_getMeasurementTimingBudget();

esp_err_t laser_range_setVcselPulsePeriod(enum laser_range_vcselPeriodType type, uint8_t period_pclks);
uint8_t laser_range_getVcselPulsePeriod(enum laser_range_vcselPeriodType type);
void laser_range_startContinuous(uint32_t period_ms);
void laser_range_stopContinuous();
uint16_t laser_range_readRangeContinuousMillimeters();
uint16_t laser_range_readRangeSingleMillimeters();

void laser_range_setTimeout(uint16_t timeout);
uint16_t laser_range_getTimeout();

uint8_t laser_range_timeoutOccurred();

esp_err_t laser_range_getSpadInfo(uint8_t * count, uint8_t * type_is_aperture);

void laser_range_getSequenceStepEnables(struct laser_range_SequenceStepEnables * enables);
void laser_range_getSequenceStepTimeouts(struct laser_range_SequenceStepEnables const * enables, struct laser_range_SequenceStepTimeouts * timeouts);

esp_err_t laser_range_performSingleRefCalibration(uint8_t vhv_init_byte);

uint16_t laser_range_decodeTimeout(uint16_t value);
uint16_t laser_range_encodeTimeout(uint32_t timeout_mclks);
uint32_t laser_range_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
uint32_t laser_range_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);

#endif /* LASER_RANGE_H_ */
