#ifndef __JY901_H__
#define __JY901_H__

#include "stm32f4xx_hal.h"   // 根据你的HAL库头文件调整
#include <stdint.h>

extern float yaw;

/* 初始化：启动UART5中断接收 */
void JY901_Init(void);

/* 设置角度参考：将当前姿态置为零度（保持静止） */
void JY901_SetZeroRef(void);

/* 此函数必须在 UART5 的中断回调中调用（见.c文件说明） */
void JY901_RxCallback(uint8_t data);

#endif
