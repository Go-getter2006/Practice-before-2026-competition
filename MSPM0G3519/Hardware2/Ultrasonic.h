#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

/*
 * 功能：初始化超声波测距驱动。
 * 说明：调用本函数前必须已经执行系统配置初始化函数。
 */
void Ultrasonic_Init(void);

/*
 * 功能：触发一次超声波测距，并等待本次测量结束。
 * 参数：距离结果指针用于保存测得的距离，单位为厘米。
 * 返回：测量成功返回一，等待回波超时或参数无效返回零。
 * 说明：本函数最长阻塞约六十毫秒，并自动保证相邻触发脉冲具有足够间隔。
 */
uint8_t Ultrasonic_MeasureDistanceCm(float *distanceCm);

#endif
