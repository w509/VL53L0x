#include "vl53l0x_user.h"
#include "stdint.h"

VL53L0X_RangingMeasurementData_t vl53l0x_data;	//测距测量结构体
volatile uint16_t Distance_data = 0;	//保存测距数据

/**
 * @brief  测量模式配置
 * @param  dev 设备I2C参数结构体
 * @param  mode 0:默认;1:高精度;2:长距离;3:高速
 * @return 返回值为错误status
 * @note   该函数使用了HAL库延时函数，会阻塞，尽量初始化后不再使用
 */
VL53L0X_Error vl53l0x_set_mode(VL53L0X_Dev_t *dev, u8 mode) {

	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	uint8_t VhvSettings;
	uint8_t PhaseCal;
	uint32_t refSpadCount;
	uint8_t isApertureSpads;

	vl53l0x_reset(dev);//复位vl53l0x(频繁切换工作模式容易导致采集距离数据不准，需加上这一代码)
	status = VL53L0X_StaticInit(dev);

	if (AjustOK != 0)//已校准好了,写入校准值
	{
		status = VL53L0X_SetReferenceSpads(dev, Vl53l0x_data.refSpadCount, Vl53l0x_data.isApertureSpads);//设定Spads校准值
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);
		status = VL53L0X_SetRefCalibration(dev, Vl53l0x_data.VhvSettings, Vl53l0x_data.PhaseCal);//设定Ref校准值
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);
		status = VL53L0X_SetOffsetCalibrationDataMicroMeter(dev, Vl53l0x_data.OffsetMicroMeter);//设定偏移校准值
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);
		status = VL53L0X_SetXTalkCompensationRateMegaCps(dev, Vl53l0x_data.XTalkCompensationRateMegaCps);//设定串扰校准值
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);

	}
	else
	{
		status = VL53L0X_PerformRefCalibration(dev, &VhvSettings, &PhaseCal);//Ref参考校准
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);
		status = VL53L0X_PerformRefSpadManagement(dev, &refSpadCount, &isApertureSpads);//执行参考SPAD管理
		if (status != VL53L0X_ERROR_NONE) goto error;
		HAL_Delay(2);
	}
	status = VL53L0X_SetDeviceMode(dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);//使能单次测量模式
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetLimitCheckEnable(dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);//使能SIGMA范围检查
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetLimitCheckEnable(dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);//使能信号速率范围检查
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetLimitCheckValue(dev, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, Mode_data[mode].sigmaLimit);//设定SIGMA范围
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetLimitCheckValue(dev, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, Mode_data[mode].signalLimit);//设定信号速率范围范围
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(dev, Mode_data[mode].timingBudget);//设定完整测距最长时间
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetVcselPulsePeriod(dev, VL53L0X_VCSEL_PERIOD_PRE_RANGE, Mode_data[mode].preRangeVcselPeriod);//设定VCSEL脉冲周期
	if (status != VL53L0X_ERROR_NONE) goto error;
	HAL_Delay(2);
	status = VL53L0X_SetVcselPulsePeriod(dev, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, Mode_data[mode].finalRangeVcselPeriod);//设定VCSEL脉冲周期范围

error://错误信息
	if (status != VL53L0X_ERROR_NONE)
	{
		print_pal_error(status);
		return status;
	}
	return status;

}

/**
 * @brief  单次距离测量函数
 * @param  dev 设备I2C参数结构体
 * @param  pdata 保存测量数据结构体
 * @return 返回距离值，单位mm
 */
VL53L0X_Error vl53l0x_start_single_test(VL53L0X_Dev_t *dev, VL53L0X_RangingMeasurementData_t *pdata) {

	VL53L0X_Error status = VL53L0X_ERROR_NONE;
	uint8_t RangeStatus;
	static char buf[VL53L0X_MAX_STRING_LENGTH];
	memset(buf, 0x00, VL53L0X_MAX_STRING_LENGTH);

	status = VL53L0X_PerformSingleRangingMeasurement(dev, pdata);//执行单次测距并获取测距测量数据
	if (status != VL53L0X_ERROR_NONE) return status;

	RangeStatus = pdata->RangeStatus;//获取当前测量状态
	VL53L0X_GetRangeStatusString(RangeStatus, buf);//根据测量状态读取状态字符串

	Distance_data = pdata->RangeMilliMeter;//保存最近一次测距测量数据

	return status;
}

/**
 * @brief  测量距离
 * @param  dev 设备I2C参数结构体
 * @return 返回距离值，单位mm
 * @note   测量成功返回距离值，失败返回9999
 */
uint16_t vl53l0x_distance(VL53L0X_Dev_t *dev) {
	static char buf[VL53L0X_MAX_STRING_LENGTH];//测试模式字符串字符缓冲区
	VL53L0X_Error Status;//工作状态
	Status = vl53l0x_start_single_test(dev, &vl53l0x_data);//执行一次测量
	if (vl53l0x_data.RangeMilliMeter <= 2000)
		return vl53l0x_data.RangeMilliMeter;
	return 9999; //返回9999表示测量失败
}