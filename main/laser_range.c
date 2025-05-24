#include "laser_range.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "flash.h"
#define LASER_RANGE_ADDRESS 0b0101001
#define laser_range_decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)
#define laser_range_encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)
#define laser_range_calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
#define laser_range_startTimeout() (laser_range_timeout_start_ms = xTaskGetTickCount()*portTICK_PERIOD_MS)
#define laser_range_checkTimeoutExpired() (laser_range_io_timeout > 0 && ((uint16_t)(xTaskGetTickCount()*portTICK_PERIOD_MS - laser_range_timeout_start_ms) > laser_range_io_timeout))

uint16_t laser_range_io_timeout;
uint8_t laser_range_did_timeout;
uint16_t laser_range_timeout_start_ms;
uint8_t laser_range_stop_variable;
uint32_t laser_range_measurement_timing_budget_us;
esp_err_t laser_range_last_status;
int16_t laser_range_calibration;

enum laser_range_regAddr
    {
      SYSRANGE_START                              = 0x00,

      SYSTEM_THRESH_HIGH                          = 0x0C,
      SYSTEM_THRESH_LOW                           = 0x0E,

      SYSTEM_SEQUENCE_CONFIG                      = 0x01,
      SYSTEM_RANGE_CONFIG                         = 0x09,
      SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

      SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

      GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

      SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

      RESULT_INTERRUPT_STATUS                     = 0x13,
      RESULT_RANGE_STATUS                         = 0x14,

      RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
      RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
      RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
      RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
      RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

      ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

      I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

      MSRC_CONFIG_CONTROL                         = 0x60,

      PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
      PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
      PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
      PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

      FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
      FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
      FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
      FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

      PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
      PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

      PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
      PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

      SYSTEM_HISTOGRAM_BIN                        = 0x81,
      HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
      HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

      FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
      FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
      CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

      MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

      SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
      IDENTIFICATION_MODEL_ID                     = 0xC0,
      IDENTIFICATION_REVISION_ID                  = 0xC2,

      OSC_CALIBRATE_VAL                           = 0xF8,

      GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
      GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

      GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
      DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
      DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
      POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

      VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

      ALGO_PHASECAL_LIM                           = 0x30,
      ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
};

esp_err_t i2c_init(){
	taskENTER_CRITICAL();
	esp_err_t err;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = 4;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = 5;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.clk_stretch_tick = 1000;
	err=i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER);
	if(err!=ESP_OK){
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error driver install: %d",err);
	    taskEXIT_CRITICAL();
		return err;
	}
	err=i2c_param_config(I2C_NUM_0, &conf);
	if(err!=ESP_OK){
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error param conf: %d",err);
	    taskEXIT_CRITICAL();
	    return err;
	}
    taskEXIT_CRITICAL();
    return ESP_OK;
}

void laser_range_writeReg(uint8_t reg, uint8_t value)
{
	taskENTER_CRITICAL();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_write_byte(cmd, value, 1);
	i2c_master_stop(cmd);
	esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C write err: %d",err);
	i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
	//last_status = bus->endTransmission();
}

void laser_range_writeReg16Bit(uint8_t reg, uint16_t value)
{
	taskENTER_CRITICAL();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_write_byte(cmd, (value>>8)&0xFF, 1);
	i2c_master_write_byte(cmd, value&0xFF, 1);
	i2c_master_stop(cmd);
	esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C write err: %d",err);
	i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
}

void laser_range_writeReg32Bit(uint8_t reg, uint32_t value)
{
	taskENTER_CRITICAL();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_write_byte(cmd, (value>>24)&0xFF, 1);
	i2c_master_write_byte(cmd, (value>>16)&0xFF, 1);
	i2c_master_write_byte(cmd, (value>>8)&0xFF, 1);
	i2c_master_write_byte(cmd, value&0xFF, 1);
	i2c_master_stop(cmd);
	esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C write err: %d",err);
	i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
}

uint8_t laser_range_readReg(uint8_t reg)
{
	taskENTER_CRITICAL();
	uint8_t value=0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
	i2c_master_read_byte(cmd, &value, I2C_MASTER_NACK);
	i2c_master_stop(cmd);
	esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	if(err!=ESP_OK)
			ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C read err: %d",err);i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
	return value;
}

uint16_t laser_range_readReg16Bit(uint8_t reg)
{
	taskENTER_CRITICAL();
	uint16_t value=0;
	void *data=&value;
	  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	  i2c_master_start(cmd);
	  i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	  i2c_master_write_byte(cmd, reg, 1);
	  i2c_master_start(cmd);
	  i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
	  i2c_master_read(cmd, data+1,1,I2C_MASTER_ACK);
	  i2c_master_read(cmd, data,1,I2C_MASTER_NACK);
	  i2c_master_stop(cmd);
	  esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	  if(err!=ESP_OK)
			ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C read err: %d",err);
	  i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
	  taskEXIT_CRITICAL();
	  return value;
}

uint32_t laser_range_readReg32Bit(uint8_t reg)
{
	taskENTER_CRITICAL();
  uint32_t value=0;
	void *data=&value;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
  i2c_master_write_byte(cmd, reg, 1);
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
  i2c_master_read(cmd, data+3,1,I2C_MASTER_ACK);
  i2c_master_read(cmd, data+2,1,I2C_MASTER_ACK);
  i2c_master_read(cmd, data+1,1,I2C_MASTER_ACK);
  i2c_master_read(cmd, data,1,I2C_MASTER_NACK);
  i2c_master_stop(cmd);
  esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
  if(err!=ESP_OK)
  		ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C read err: %d",err);
  i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
	  taskEXIT_CRITICAL();
  return value;
}

void laser_range_writeMulti(uint8_t reg, uint8_t * src, uint8_t count)
{
	taskENTER_CRITICAL();
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_write(cmd, src,count, 1);
	i2c_master_stop(cmd);
	esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C write err: %d",err);
	i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
}

void laser_range_readMulti(uint8_t reg, uint8_t * dst, uint8_t count)
{
	taskENTER_CRITICAL();
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
    i2c_master_read(cmd, dst,count-1,I2C_MASTER_ACK);
    i2c_master_read(cmd, dst+count-1,1,I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
    if(err!=ESP_OK)
    	ESP_LOGE(LASER_RANGE_LOG_TAG,"I2C read err: %d",err);    i2c_cmd_link_delete(cmd);
	laser_range_last_status=err;
    taskEXIT_CRITICAL();
}

uint16_t laser_range_encodeTimeout(uint32_t timeout_mclks)
{
  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

uint16_t laser_range_decodeTimeout(uint16_t reg_val)
{
  return (uint16_t)((reg_val & 0x00FF) <<
         (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

uint32_t laser_range_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = laser_range_calcMacroPeriod(vcsel_period_pclks);

  return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

uint32_t laser_range_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = laser_range_calcMacroPeriod(vcsel_period_pclks);

  return ((timeout_period_mclks * macro_period_ns) + 500) / 1000;
}

void laser_range_getSequenceStepTimeouts(struct laser_range_SequenceStepEnables const * enables,struct laser_range_SequenceStepTimeouts * timeouts)
{
  timeouts->pre_range_vcsel_period_pclks = laser_range_getVcselPulsePeriod(VcselPeriodPreRange);

  timeouts->msrc_dss_tcc_mclks = laser_range_readReg(MSRC_CONFIG_TIMEOUT_MACROP) + 1;
  timeouts->msrc_dss_tcc_us =
    laser_range_timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->pre_range_mclks =laser_range_decodeTimeout(laser_range_readReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
  timeouts->pre_range_us =laser_range_timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->final_range_vcsel_period_pclks = laser_range_getVcselPulsePeriod(VcselPeriodFinalRange);

  timeouts->final_range_mclks =
    laser_range_decodeTimeout(laser_range_readReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

  if (enables->pre_range)
  {
    timeouts->final_range_mclks -= timeouts->pre_range_mclks;
  }

  timeouts->final_range_us =
		  laser_range_timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                               timeouts->final_range_vcsel_period_pclks);
}

void laser_range_getSequenceStepEnables(struct laser_range_SequenceStepEnables * enables)
{
  uint8_t sequence_config = laser_range_readReg(SYSTEM_SEQUENCE_CONFIG);

  enables->tcc          = (sequence_config >> 4) & 0x1;
  enables->dss          = (sequence_config >> 3) & 0x1;
  enables->msrc         = (sequence_config >> 2) & 0x1;
  enables->pre_range    = (sequence_config >> 6) & 0x1;
  enables->final_range  = (sequence_config >> 7) & 0x1;
}

uint8_t laser_range_timeoutOccurred()
{
  uint8_t tmp = laser_range_did_timeout;
  laser_range_did_timeout = 0;
  return tmp;
}

void laser_range_startContinuous(uint32_t period_ms)
{
	laser_range_writeReg(0x80, 0x01);
	laser_range_writeReg(0xFF, 0x01);
	laser_range_writeReg(0x00, 0x00);
	laser_range_writeReg(0x91, laser_range_stop_variable);
	laser_range_writeReg(0x00, 0x01);
	laser_range_writeReg(0xFF, 0x00);
	laser_range_writeReg(0x80, 0x00);

  if (period_ms != 0)
  {
   uint16_t osc_calibrate_val = laser_range_readReg16Bit(OSC_CALIBRATE_VAL);

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }

    laser_range_writeReg32Bit(SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);
    laser_range_writeReg(SYSRANGE_START, 0x04);
  }
  else
  {
	  laser_range_writeReg(SYSRANGE_START, 0x02);
  }
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Started continious");
}

esp_err_t laser_range_performSingleRefCalibration(uint8_t vhv_init_byte)
{
  laser_range_writeReg(SYSRANGE_START, 0x01 | vhv_init_byte);
  laser_range_startTimeout();
  while ((laser_range_readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
  {
    if (laser_range_checkTimeoutExpired()) {
  	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Error: timeout");
    	return ESP_FAIL;
    }
  }

  laser_range_writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

  laser_range_writeReg(SYSRANGE_START, 0x00);
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range perform single ref calibration sucsessful");
  return ESP_OK;
}

esp_err_t laser_range_getSpadInfo(uint8_t * count, uint8_t * type_is_aperture)
{
  uint8_t tmp;

  laser_range_writeReg(0x80, 0x01);
  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x00, 0x00);

  laser_range_writeReg(0xFF, 0x06);
  laser_range_writeReg(0x83,laser_range_readReg(0x83) | 0x04);
  laser_range_writeReg(0xFF, 0x07);
  laser_range_writeReg(0x81, 0x01);

  laser_range_writeReg(0x80, 0x01);

  laser_range_writeReg(0x94, 0x6b);
  laser_range_writeReg(0x83, 0x00);
  laser_range_startTimeout();
  while (laser_range_readReg(0x83) == 0x00)
  {
    if (laser_range_checkTimeoutExpired()) {
  	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Error: timeout");
    	return ESP_FAIL;
    }
  }
  laser_range_writeReg(0x83, 0x01);
  tmp = laser_range_readReg(0x92);

  *count = tmp & 0x7f;
  *type_is_aperture = (tmp >> 7) & 0x01;

  laser_range_writeReg(0x81, 0x00);
  laser_range_writeReg(0xFF, 0x06);
  laser_range_writeReg(0x83, laser_range_readReg(0x83)  & ~0x04);
  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x00, 0x01);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x80, 0x00);
  return ESP_OK;
}

uint16_t laser_range_readRangeSingleMillimeters()
{
	  ESP_LOGI(LASER_RANGE_LOG_TAG,"Start reading single");
	laser_range_writeReg(0x80, 0x01);
	laser_range_writeReg(0xFF, 0x01);
	laser_range_writeReg(0x00, 0x00);
	laser_range_writeReg(0x91, laser_range_stop_variable);
	laser_range_writeReg(0x00, 0x01);
	laser_range_writeReg(0xFF, 0x00);
	laser_range_writeReg(0x80, 0x00);

	laser_range_writeReg(SYSRANGE_START, 0x01);
  laser_range_startTimeout();
  while (laser_range_readReg(SYSRANGE_START) & 0x01)
  {
    if (laser_range_checkTimeoutExpired())
    {
    	laser_range_did_timeout = 1;
    	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Error: timeout");
    	  return 65535;
    }
  }

  return laser_range_readRangeContinuousMillimeters();
}

uint16_t laser_range_readRangeContinuousMillimeters()
{
	  ESP_LOGI(LASER_RANGE_LOG_TAG,"Start reading continious");
	laser_range_startTimeout();
  while ((laser_range_readReg(RESULT_INTERRUPT_STATUS) & 0x07) == 0)
  {
    if (laser_range_checkTimeoutExpired())
    {
    	laser_range_did_timeout = 1;
    	ESP_LOGE(LASER_RANGE_LOG_TAG,"Error: timeout");
    	return 65535;
    }
  }

  uint16_t range = laser_range_readReg16Bit(RESULT_RANGE_STATUS + 10);
  laser_range_writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);
  range+=laser_range_calibration;
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Read range: %d",range);
  return range;
}

void laser_range_stopContinuous()
{
	laser_range_writeReg(SYSRANGE_START, 0x01);
	laser_range_writeReg(0xFF, 0x01);
	laser_range_writeReg(0x00, 0x00);
	laser_range_writeReg(0x91, 0x00);
	laser_range_writeReg(0x00, 0x01);
	laser_range_writeReg(0xFF, 0x00);
	ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range stoped continious");
}

uint8_t laser_range_getVcselPulsePeriod(enum laser_range_vcselPeriodType type)
{
  if (type == VcselPeriodPreRange)
  {
    return laser_range_decodeVcselPeriod(laser_range_readReg(PRE_RANGE_CONFIG_VCSEL_PERIOD));
  }
  else if (type == VcselPeriodFinalRange)
  {
    return laser_range_decodeVcselPeriod(laser_range_readReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD));
  }
  else { return 255; }
}

esp_err_t laser_range_setVcselPulsePeriod(enum laser_range_vcselPeriodType type, uint8_t period_pclks)
{
  uint8_t vcsel_period_reg =  laser_range_encodeVcselPeriod(period_pclks);
  struct laser_range_SequenceStepEnables enables;
  struct laser_range_SequenceStepTimeouts timeouts;
  laser_range_getSequenceStepEnables(&enables);
  laser_range_getSequenceStepTimeouts(&enables, &timeouts);
  if (type == VcselPeriodPreRange)
  {
    switch (period_pclks)
    {
      case 12:
    	laser_range_writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x18);
        break;
      case 14:
    	laser_range_writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x30);
        break;
      case 16:
    	laser_range_writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x40);
        break;
      case 18:
    	laser_range_writeReg(PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x50);
        break;
      default:
    	ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range set vcsel pulse period error");
        return ESP_FAIL;
    }
    laser_range_writeReg(PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08);
    laser_range_writeReg(PRE_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
    uint16_t new_pre_range_timeout_mclks = laser_range_timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);
    laser_range_writeReg16Bit(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,
    laser_range_encodeTimeout(new_pre_range_timeout_mclks));
    uint16_t new_msrc_timeout_mclks = laser_range_timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);
    laser_range_writeReg(MSRC_CONFIG_TIMEOUT_MACROP,(new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1));
  }
  else if (type == VcselPeriodFinalRange)
  {
    switch (period_pclks)
    {
      case 8:
    	  laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x10);
    	  laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
    	  laser_range_writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x02);
    	  laser_range_writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x0C);
    	  laser_range_writeReg(0xFF, 0x01);
    	  laser_range_writeReg(ALGO_PHASECAL_LIM, 0x30);
    	  laser_range_writeReg(0xFF, 0x00);
        break;
      case 10:
    	laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x28);
        laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        laser_range_writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        laser_range_writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x09);
        laser_range_writeReg(0xFF, 0x01);
        laser_range_writeReg(ALGO_PHASECAL_LIM, 0x20);
        laser_range_writeReg(0xFF, 0x00);
        break;
      case 12:
        laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x38);
        laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        laser_range_writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        laser_range_writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x08);
        laser_range_writeReg(0xFF, 0x01);
        laser_range_writeReg(ALGO_PHASECAL_LIM, 0x20);
        laser_range_writeReg(0xFF, 0x00);
        break;
      case 14:
        laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_HIGH, 0x48);
        laser_range_writeReg(FINAL_RANGE_CONFIG_VALID_PHASE_LOW,  0x08);
        laser_range_writeReg(GLOBAL_CONFIG_VCSEL_WIDTH, 0x03);
        laser_range_writeReg(ALGO_PHASECAL_CONFIG_TIMEOUT, 0x07);
        laser_range_writeReg(0xFF, 0x01);
        laser_range_writeReg(ALGO_PHASECAL_LIM, 0x20);
        laser_range_writeReg(0xFF, 0x00);
        break;
      default:
    	ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range set vcsel pulse period error");
        return ESP_FAIL;
    }

    laser_range_writeReg(FINAL_RANGE_CONFIG_VCSEL_PERIOD, vcsel_period_reg);
    uint16_t new_final_range_timeout_mclks = laser_range_timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);
    if (enables.pre_range)
    {
      new_final_range_timeout_mclks += timeouts.pre_range_mclks;
    }
    laser_range_writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,laser_range_encodeTimeout(new_final_range_timeout_mclks));
  } else{
	ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range set vcsel pulse period error");
    return ESP_FAIL;
  }
  laser_range_setMeasurementTimingBudget(laser_range_measurement_timing_budget_us);
  uint8_t sequence_config = laser_range_readReg(SYSTEM_SEQUENCE_CONFIG);
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
  laser_range_performSingleRefCalibration(0x0);
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, sequence_config);
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range set vcsel pulse period sucsessful");
  return ESP_OK;
}

uint32_t laser_range_getMeasurementTimingBudget(){
	struct laser_range_SequenceStepEnables enables;
 struct laser_range_SequenceStepTimeouts timeouts;
  uint16_t const StartOverhead     = 1910;
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t budget_us = StartOverhead + EndOverhead;

  laser_range_getSequenceStepEnables(&enables);
  laser_range_getSequenceStepTimeouts(&enables, &timeouts);

  if (enables.tcc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    budget_us += (timeouts.final_range_us + FinalRangeOverhead);
  }
  laser_range_measurement_timing_budget_us = budget_us;
  return budget_us;
}

esp_err_t laser_range_setMeasurementTimingBudget(uint32_t budget_us)
{
	struct	laser_range_SequenceStepEnables enables;
	struct laser_range_SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead     = 1910;
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t used_budget_us = StartOverhead + EndOverhead;

  laser_range_getSequenceStepEnables(&enables);
  laser_range_getSequenceStepTimeouts(&enables, &timeouts);

  if (enables.tcc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    used_budget_us += FinalRangeOverhead;
    if (used_budget_us > budget_us)
    {
      ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range set measurement timing budget error");
      return ESP_FAIL;
    }

    uint32_t final_range_timeout_us = budget_us - used_budget_us;
    uint32_t final_range_timeout_mclks =laser_range_timeoutMicrosecondsToMclks(final_range_timeout_us, timeouts.final_range_vcsel_period_pclks);

    if (enables.pre_range)
    {
      final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    laser_range_writeReg16Bit(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
    laser_range_encodeTimeout(final_range_timeout_mclks));

    laser_range_measurement_timing_budget_us = budget_us;
  }
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range set measurement timing budget sucsessful");
  return ESP_OK;
}

esp_err_t laser_range_setSignalRateLimit(float limit_Mcps)
{
  if (limit_Mcps < 0 || limit_Mcps > 511.99) {
	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range set signal rate limit error");
	  return ESP_FAIL;
  }
  laser_range_writeReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7));
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range set signal rate limit sucsessful");
  return ESP_OK;
}

float laser_range_getSignalRateLimit()
{
  return (float)laser_range_readReg16Bit(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT) / (1 << 7);
}

void laser_range_setTimeout(uint16_t timeout) { laser_range_io_timeout = timeout; }
uint16_t laser_range_getTimeout() { return laser_range_io_timeout; }
esp_err_t laser_range_get_last_status(){return laser_range_last_status;}

esp_err_t laser_range_init(uint8_t io_2v8)
{
	laser_range_io_timeout=0;
	laser_range_did_timeout=0;
  if (laser_range_readReg(IDENTIFICATION_MODEL_ID) != 0xEE) {
	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range init error");
	  return ESP_FAIL;
  }
  if (io_2v8)
  {
    laser_range_writeReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
    laser_range_readReg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01);
  }
  laser_range_writeReg(0x88, 0x00);
  laser_range_writeReg(0x80, 0x01);
  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x00, 0x00);
  laser_range_stop_variable = laser_range_readReg(0x91);
  laser_range_writeReg(0x00, 0x01);
  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x80, 0x00);
  laser_range_writeReg(MSRC_CONFIG_CONTROL, laser_range_readReg(MSRC_CONFIG_CONTROL) | 0x12);
  laser_range_setSignalRateLimit(0.25);
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0xFF);
  uint8_t spad_count;
  uint8_t spad_type_is_aperture;
  if (laser_range_getSpadInfo(&spad_count, &spad_type_is_aperture)!=ESP_OK) {
	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range init error");
	  return ESP_FAIL;
  }
  uint8_t ref_spad_map[6];
  laser_range_readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
  laser_range_writeReg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);
  uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0;
  uint8_t spads_enabled = 0;
  for (uint8_t i = 0; i < 48; i++)
  {
    if (i < first_spad_to_enable || spads_enabled == spad_count)
    {
      ref_spad_map[i / 8] &= ~(1 << (i % 8));
    }
    else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
    {
      spads_enabled++;
    }
  }

  laser_range_writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x00, 0x00);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x09, 0x00);
  laser_range_writeReg(0x10, 0x00);
  laser_range_writeReg(0x11, 0x00);

  laser_range_writeReg(0x24, 0x01);
  laser_range_writeReg(0x25, 0xFF);
  laser_range_writeReg(0x75, 0x00);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x4E, 0x2C);
  laser_range_writeReg(0x48, 0x00);
  laser_range_writeReg(0x30, 0x20);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x30, 0x09);
  laser_range_writeReg(0x54, 0x00);
  laser_range_writeReg(0x31, 0x04);
  laser_range_writeReg(0x32, 0x03);
  laser_range_writeReg(0x40, 0x83);
  laser_range_writeReg(0x46, 0x25);
  laser_range_writeReg(0x60, 0x00);
  laser_range_writeReg(0x27, 0x00);
  laser_range_writeReg(0x50, 0x06);
  laser_range_writeReg(0x51, 0x00);
  laser_range_writeReg(0x52, 0x96);
  laser_range_writeReg(0x56, 0x08);
  laser_range_writeReg(0x57, 0x30);
  laser_range_writeReg(0x61, 0x00);
  laser_range_writeReg(0x62, 0x00);
  laser_range_writeReg(0x64, 0x00);
  laser_range_writeReg(0x65, 0x00);
  laser_range_writeReg(0x66, 0xA0);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x22, 0x32);
  laser_range_writeReg(0x47, 0x14);
  laser_range_writeReg(0x49, 0xFF);
  laser_range_writeReg(0x4A, 0x00);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x7A, 0x0A);
  laser_range_writeReg(0x7B, 0x00);
  laser_range_writeReg(0x78, 0x21);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x23, 0x34);
  laser_range_writeReg(0x42, 0x00);
  laser_range_writeReg(0x44, 0xFF);
  laser_range_writeReg(0x45, 0x26);
  laser_range_writeReg(0x46, 0x05);
  laser_range_writeReg(0x40, 0x40);
  laser_range_writeReg(0x0E, 0x06);
  laser_range_writeReg(0x20, 0x1A);
  laser_range_writeReg(0x43, 0x40);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x34, 0x03);
  laser_range_writeReg(0x35, 0x44);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x31, 0x04);
  laser_range_writeReg(0x4B, 0x09);
  laser_range_writeReg(0x4C, 0x05);
  laser_range_writeReg(0x4D, 0x04);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x44, 0x00);
  laser_range_writeReg(0x45, 0x20);
  laser_range_writeReg(0x47, 0x08);
  laser_range_writeReg(0x48, 0x28);
  laser_range_writeReg(0x67, 0x00);
  laser_range_writeReg(0x70, 0x04);
  laser_range_writeReg(0x71, 0x01);
  laser_range_writeReg(0x72, 0xFE);
  laser_range_writeReg(0x76, 0x00);
  laser_range_writeReg(0x77, 0x00);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x0D, 0x01);

  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x80, 0x01);
  laser_range_writeReg(0x01, 0xF8);

  laser_range_writeReg(0xFF, 0x01);
  laser_range_writeReg(0x8E, 0x01);
  laser_range_writeReg(0x00, 0x01);
  laser_range_writeReg(0xFF, 0x00);
  laser_range_writeReg(0x80, 0x00);


  laser_range_writeReg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
  laser_range_writeReg(GPIO_HV_MUX_ACTIVE_HIGH, laser_range_readReg(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10);
  laser_range_writeReg(SYSTEM_INTERRUPT_CLEAR, 0x01);

  laser_range_measurement_timing_budget_us = laser_range_getMeasurementTimingBudget();
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
  laser_range_setMeasurementTimingBudget(laser_range_measurement_timing_budget_us);
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0x01);
  if (laser_range_performSingleRefCalibration(0x40)!=ESP_OK) {
	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range init error");
	  return ESP_FAIL;
  }
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0x02);
  if (laser_range_performSingleRefCalibration(0x00)!=ESP_OK) {
	  ESP_LOGE(LASER_RANGE_LOG_TAG,"Laser range init error");
	  return ESP_FAIL;
  }
  laser_range_writeReg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
  if(load_range_calibration_from_flash(&laser_range_calibration)!=ESP_OK)
	  laser_range_calibration=0;
  ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range init sucsessful");
  return ESP_OK;
}


