#ifndef INC_MOTION_H_
#define INC_MOTION_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include "Delay.h"

// 传感器通道总数
#define GRAYSCALE_SENSOR_CHANNELS 8

// 巡线传感器初始化
void Grayscale_Sensor_Init(void);
// 读取所有8个通道的灰度值
void Grayscale_Sensor_Read_All(uint16_t* sensor_values);
// 读取单个指定通道的灰度值
uint16_t Grayscale_Sensor_Read_Single(uint8_t channel);

// 获取灰度传感器状态
uint8_t Get_Grayscale_State(void);
// 全局巡线误差
extern float error;
// 巡线误差计算
float Track_err(void);
// 巡线PID控制
int PID_out(float error, int Target);

#endif
