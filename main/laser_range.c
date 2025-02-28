#include "laser_range.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

enum VL53L0X_regAddr
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

void laser_range_init(){
	esp_err_t err;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = 4;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = 5;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.clk_stretch_tick = 300;
	err=i2c_driver_install(I2C_NUM_0,I2C_MODE_MASTER);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error driver install: %d",err);
	err=i2c_param_config(I2C_NUM_0, &conf);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error param conf: %d",err);

	uint8_t buff;
	err=read_byte_from_laser_range(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,&buff);
	buff|=0x01;
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C read: %d",err);
	write_byte_to_laser_range(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,0x66);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C write: %d",err);
	write_byte_to_laser_range(0x88, 0x00);
	write_byte_to_laser_range(0x80, 0x01);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x00, 0x00);
	read_byte_from_laser_range(0x91,&laser_range_stop_variable);
	write_byte_to_laser_range(0x00, 0x01);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x80, 0x00);
	read_byte_from_laser_range(MSRC_CONFIG_CONTROL,&buff);
	buff|=0x12;
	write_byte_to_laser_range(MSRC_CONFIG_CONTROL,buff);
	laser_range_setSignalRateLimit(0.25);
	write_byte_to_laser_range(SYSTEM_SEQUENCE_CONFIG, 0xFF);
	uint8_t spad_count;
	uint8_t spad_type_is_aperture;
	laser_range_getSpadInfo(&spad_count, &spad_type_is_aperture);
	uint8_t ref_spad_map[6];
	read_from_laser_range(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
	write_byte_to_laser_range(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);
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
	write_to_laser_range(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x00, 0x00);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x09, 0x00);
	write_byte_to_laser_range(0x10, 0x00);
	write_byte_to_laser_range(0x11, 0x00);
	write_byte_to_laser_range(0x24, 0x01);
	write_byte_to_laser_range(0x25, 0xFF);
	write_byte_to_laser_range(0x75, 0x00);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x4E, 0x2C);
	write_byte_to_laser_range(0x48, 0x00);
	write_byte_to_laser_range(0x30, 0x20);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x30, 0x09);
	write_byte_to_laser_range(0x54, 0x00);
	write_byte_to_laser_range(0x31, 0x04);
	write_byte_to_laser_range(0x32, 0x03);
	write_byte_to_laser_range(0x40, 0x83);
	write_byte_to_laser_range(0x46, 0x25);
	write_byte_to_laser_range(0x60, 0x00);
	write_byte_to_laser_range(0x27, 0x00);
	write_byte_to_laser_range(0x50, 0x06);
	write_byte_to_laser_range(0x51, 0x00);
	write_byte_to_laser_range(0x52, 0x96);
	write_byte_to_laser_range(0x56, 0x08);
	write_byte_to_laser_range(0x57, 0x30);
	write_byte_to_laser_range(0x61, 0x00);
	write_byte_to_laser_range(0x62, 0x00);
	write_byte_to_laser_range(0x64, 0x00);
	write_byte_to_laser_range(0x65, 0x00);
	write_byte_to_laser_range(0x66, 0xA0);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x22, 0x32);
	write_byte_to_laser_range(0x47, 0x14);
	write_byte_to_laser_range(0x49, 0xFF);
	write_byte_to_laser_range(0x4A, 0x00);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x7A, 0x0A);
	write_byte_to_laser_range(0x7B, 0x00);
	write_byte_to_laser_range(0x78, 0x21);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x23, 0x34);
	write_byte_to_laser_range(0x42, 0x00);
	write_byte_to_laser_range(0x44, 0xFF);
	write_byte_to_laser_range(0x45, 0x26);
	write_byte_to_laser_range(0x46, 0x05);
	write_byte_to_laser_range(0x40, 0x40);
	write_byte_to_laser_range(0x0E, 0x06);
	write_byte_to_laser_range(0x20, 0x1A);
	write_byte_to_laser_range(0x43, 0x40);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x34, 0x03);
	write_byte_to_laser_range(0x35, 0x44);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x31, 0x04);
	write_byte_to_laser_range(0x4B, 0x09);
	write_byte_to_laser_range(0x4C, 0x05);
	write_byte_to_laser_range(0x4D, 0x04);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x44, 0x00);
	write_byte_to_laser_range(0x45, 0x20);
	write_byte_to_laser_range(0x47, 0x08);
	write_byte_to_laser_range(0x48, 0x28);
	write_byte_to_laser_range(0x67, 0x00);
	write_byte_to_laser_range(0x70, 0x04);
	write_byte_to_laser_range(0x71, 0x01);
	write_byte_to_laser_range(0x72, 0xFE);
	write_byte_to_laser_range(0x76, 0x00);
	write_byte_to_laser_range(0x77, 0x00);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x0D, 0x01);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x80, 0x01);
	write_byte_to_laser_range(0x01, 0xF8);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x8E, 0x01);
	write_byte_to_laser_range(0x00, 0x01);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x80, 0x00);
	write_byte_to_laser_range(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
	read_byte_from_laser_range(GPIO_HV_MUX_ACTIVE_HIGH,&buff);
	buff&= ~0x10;
	write_byte_to_laser_range(GPIO_HV_MUX_ACTIVE_HIGH,buff);
	write_byte_to_laser_range(SYSTEM_INTERRUPT_CLEAR, 0x01);
	laser_range_measurement_timing_budget_us = laser_range_getMeasurementTimingBudget();
	write_byte_to_laser_range(SYSTEM_SEQUENCE_CONFIG, 0xE8);
	laser_range_setMeasurementTimingBudget(laser_range_measurement_timing_budget_us);
	write_byte_to_laser_range(SYSTEM_SEQUENCE_CONFIG, 0x01);
	laser_range_performSingleRefCalibration(0x40);
	write_byte_to_laser_range(SYSTEM_SEQUENCE_CONFIG, 0x02);
	laser_range_performSingleRefCalibration(0x00);
	err=write_byte_to_laser_range(SYSTEM_SEQUENCE_CONFIG, 0xE8);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C write: %d",err);
	ESP_LOGI(LASER_RANGE_LOG_TAG,"Laser range initalized");
}

esp_err_t write_byte_to_laser_range(uint8_t address,uint8_t byte){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, address, 1);
    i2c_master_write_byte(cmd, byte, 1);
    i2c_master_stop(cmd);
    esp_err_t ret=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t write_to_laser_range(uint8_t address,void *data,int l){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1 )|I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, address, 1);
    i2c_master_write(cmd, data,l, 1);
    i2c_master_stop(cmd);
    esp_err_t ret=i2c_master_cmd_begin(I2C_NUM_0,cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t read_byte_from_laser_range(uint8_t address,uint8_t *byte){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, address, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
    i2c_master_read_byte(cmd, byte, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t read_from_laser_range(uint8_t address,void *data,int l){
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LASER_RANGE_ADDRESS << 1) |I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, address, 1);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, LASER_RANGE_ADDRESS << 1 | I2C_MASTER_READ, 1);
    i2c_master_read(cmd, data,l-1,I2C_MASTER_ACK);
    i2c_master_read(cmd, data+l-1,1,I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}
uint8_t laser_range_setSignalRateLimit(float limit_Mcps)
{
	if (limit_Mcps < 0 || limit_Mcps > 511.99) { return 0; }
	limit_Mcps*= (1 << 7);
	write_to_laser_range(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,&limit_Mcps,2);
	return 1;
}

void laser_range_getSpadInfo(uint8_t * count, uint8_t * type_is_aperture)
{
	uint8_t tmp=0x00;
	write_byte_to_laser_range(0x80, 0x01);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x00, 0x00);
	write_byte_to_laser_range(0xFF, 0x06);
	read_byte_from_laser_range(0x83,&tmp);
	tmp|=0x04;
	write_byte_to_laser_range(0x83, tmp);
	write_byte_to_laser_range(0xFF, 0x07);
	write_byte_to_laser_range(0x81, 0x01);
	write_byte_to_laser_range(0x80, 0x01);
	write_byte_to_laser_range(0x94, 0x6b);
	write_byte_to_laser_range(0x83, 0x00);
	while (true){
		esp_err_t err=read_byte_from_laser_range(0x83,&tmp);
		if(tmp!=0x00||err!=ESP_OK)
			break;
	}
	write_byte_to_laser_range(0x83, 0x01);
	read_byte_from_laser_range(0x92,&tmp);
	*count = tmp & 0x7f;
	*type_is_aperture = (tmp >> 7) & 0x01;
	write_byte_to_laser_range(0x81, 0x00);
	write_byte_to_laser_range(0xFF, 0x06);
	read_byte_from_laser_range(0x83,&tmp);
	tmp&=~0x04;
	write_byte_to_laser_range(0x83,tmp);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x00, 0x01);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x80, 0x00);
}

uint32_t laser_range_getMeasurementTimingBudget()
{
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

void laser_range_setMeasurementTimingBudget(uint32_t budget_us)
{
	struct laser_range_SequenceStepEnables enables;
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

		uint32_t final_range_timeout_us = budget_us - used_budget_us;
		uint32_t final_range_timeout_mclks =
		laser_range_timeoutMicrosecondsToMclks(final_range_timeout_us,
		timeouts.final_range_vcsel_period_pclks);

		if (enables.pre_range)
		{
			final_range_timeout_mclks += timeouts.pre_range_mclks;
		}
		uint16_t buff=laser_range_encodeTimeout(final_range_timeout_mclks);
		write_to_laser_range(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,&buff,2);
		laser_range_measurement_timing_budget_us = budget_us;
	}
}
void laser_range_getSequenceStepEnables(struct laser_range_SequenceStepEnables * enables)
{
	uint8_t sequence_config;
	read_byte_from_laser_range(SYSTEM_SEQUENCE_CONFIG,&sequence_config);
	enables->tcc          = (sequence_config >> 4) & 0x1;
	enables->dss          = (sequence_config >> 3) & 0x1;
	enables->msrc         = (sequence_config >> 2) & 0x1;
	enables->pre_range    = (sequence_config >> 6) & 0x1;
	enables->final_range  = (sequence_config >> 7) & 0x1;
}

void laser_range_getSequenceStepTimeouts(struct laser_range_SequenceStepEnables const * enables,struct laser_range_SequenceStepTimeouts * timeouts)
{
	timeouts->pre_range_vcsel_period_pclks = laser_range_getVcselPulsePeriod(VcselPeriodPreRange);

	read_byte_from_laser_range(MSRC_CONFIG_TIMEOUT_MACROP,(uint8_t*)&(timeouts->msrc_dss_tcc_mclks));
	timeouts->msrc_dss_tcc_mclks+=1;
	timeouts->msrc_dss_tcc_us =
	laser_range_timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
	timeouts->pre_range_vcsel_period_pclks);
	uint16_t buff;
	read_from_laser_range(PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI,&buff,2);
	timeouts->pre_range_mclks =	laser_range_decodeTimeout(buff);
	timeouts->pre_range_us =
	laser_range_timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
	timeouts->pre_range_vcsel_period_pclks);

	timeouts->final_range_vcsel_period_pclks = laser_range_getVcselPulsePeriod(VcselPeriodFinalRange);
	read_from_laser_range(FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,&buff,2);
	timeouts->final_range_mclks =laser_range_decodeTimeout(buff);

	if (enables->pre_range)
	{
		timeouts->final_range_mclks -= timeouts->pre_range_mclks;
	}

	timeouts->final_range_us =laser_range_timeoutMclksToMicroseconds(timeouts->final_range_mclks,
										timeouts->final_range_vcsel_period_pclks);
}

uint8_t laser_range_getVcselPulsePeriod(enum laser_range_vcselPeriodType type)
{
	if (type == VcselPeriodPreRange)
	{
		uint8_t reg_val;
		read_byte_from_laser_range(PRE_RANGE_CONFIG_VCSEL_PERIOD,&reg_val);
		return  (((reg_val) + 1) << 1);
	}
	else if (type == VcselPeriodFinalRange)
	{
		uint8_t reg_val;
		read_byte_from_laser_range(FINAL_RANGE_CONFIG_VCSEL_PERIOD,&reg_val);
		return  (((reg_val) + 1) << 1);
	}
	else { return 255; }
}

uint32_t laser_range_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000);

	return ((timeout_period_mclks * macro_period_ns) + 500) / 1000;
}
uint32_t laser_range_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
	uint32_t macro_period_ns = ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000);

	return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

uint16_t laser_range_decodeTimeout(uint16_t reg_val)
{
	return (uint16_t)((reg_val & 0x00FF) <<
	(uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

void laser_range_performSingleRefCalibration(uint8_t vhv_init_byte)
{
	write_byte_to_laser_range(SYSRANGE_START, 0x01 | vhv_init_byte);

	uint8_t tmp;
	while (true){
		esp_err_t err=read_byte_from_laser_range(RESULT_INTERRUPT_STATUS,&tmp);
		tmp&=0x07;
		if(tmp==0x00||err!=ESP_OK)
			break;
	}
	write_byte_to_laser_range(SYSTEM_INTERRUPT_CLEAR, 0x01);

	write_byte_to_laser_range(SYSRANGE_START, 0x00);
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

void laser_range_askRangeSingleMillimeters()
{
	ESP_LOGI(LASER_RANGE_LOG_TAG,"Scan asked");
	esp_err_t err=write_byte_to_laser_range(0x80, 0x01);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C write: %d",err);
	write_byte_to_laser_range(0xFF, 0x01);
	write_byte_to_laser_range(0x00, 0x00);
	write_byte_to_laser_range(0x91, laser_range_stop_variable);
	write_byte_to_laser_range(0x00, 0x01);
	write_byte_to_laser_range(0xFF, 0x00);
	write_byte_to_laser_range(0x80, 0x00);
	write_byte_to_laser_range(SYSRANGE_START, 0x01);
}

uint16_t laser_range_readRangeSingleMillimeters()
{
	ESP_LOGI(LASER_RANGE_LOG_TAG,"Start reading");
	uint8_t tmp;
	esp_err_t err;
	while (true){
		err=read_byte_from_laser_range(SYSRANGE_START,&tmp);
		tmp&=0x01;
		if(err!=ESP_OK){
			ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C read: %d",err);
			return 0xFFFF;
		}
		if(!tmp)
			break;
	}
	while (true){
		   err=read_byte_from_laser_range(RESULT_INTERRUPT_STATUS,&tmp);
			tmp&=0x07;
			if(err!=ESP_OK){
				ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C read: %d",err);
				return 0xFFFF;
			}
			if(tmp!=0x00)
				break;
		}
	uint16_t range;
	read_from_laser_range(RESULT_RANGE_STATUS + 10,&range,2);
	err=write_byte_to_laser_range(SYSTEM_INTERRUPT_CLEAR, 0x01);
	if(err!=ESP_OK)
		ESP_LOGE(LASER_RANGE_LOG_TAG,"Error I2C write: %d",err);
	ESP_LOGI(LASER_RANGE_LOG_TAG,"Scaned range %d",range);
	return range;

}
