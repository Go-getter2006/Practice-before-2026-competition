#ifndef HARDWARE_LIDAR_H_
#define HARDWARE_LIDAR_H_

#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief N10 小车目标识别结果。
  * @note  car_angle 遵循雷达坐标系，范围为 0°～359.99°；仅当
  *        car_angle_valid 为 1 时，角度、距离和点数有效。
  */
extern volatile float car_angle;
extern volatile uint8_t car_angle_valid;
extern volatile uint16_t car_distance_mm;
extern volatile uint16_t lidar_target_point_count;
extern volatile uint32_t lidar_valid_frame_count;
extern volatile uint32_t lidar_error_frame_count;

/**
  * @brief  初始化雷达解析状态并启动 USART1 单字节中断接收。
  */
void Lidar_Init(void);

/**
  * @brief  在主循环中处理已经接收完成的一整圈数据。
  */
void Lidar_Process(void);

/**
  * @brief  USART1 接收完成回调入口。
  * @note   由统一的 HAL 串口接收完成回调调用。
  */
void Lidar_UART_RxCpltCallback(void);

/**
  * @brief  USART1 接收错误恢复入口。
  */
void Lidar_UART_ErrorCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* 雷达驱动头文件 */
