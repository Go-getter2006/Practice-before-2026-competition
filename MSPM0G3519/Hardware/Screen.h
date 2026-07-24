#ifndef SCREEN_H
#define SCREEN_H

#include <stdbool.h>
#include <stdint.h>

/* 串口屏通信帧的固定字节。 */
#define SCREEN_FRAME_HEADER    (0xFFU)
#define SCREEN_FRAME_TAIL      (0xFEU)

/* 当前支持的任务编号。 */
#define SCREEN_TASK_1          (0x01U)
#define SCREEN_TASK_2          (0x02U)
#define SCREEN_TASK_3          (0x03U)

/*******************************************************************************
 * 名    称：Screen_Init
 * 功    能：初始化串口屏接收状态，并开启 UART_Screen 接收中断。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：调用本函数前必须先完成系统配置初始化。
 *******************************************************************************/
void Screen_Init(void);

/*******************************************************************************
 * 名    称：Screen_ReadTask
 * 功    能：从串口屏任务队列中读取一个待执行任务号。
 * 参    数：taskNumber：用于保存任务号的指针。
 * 出    口：读取成功返回真，当前没有任务或指针为空时返回假。
 * 说    明：本函数应在主循环中调用，任务不会在串口中断中直接执行。
 *******************************************************************************/
bool Screen_ReadTask(uint8_t *taskNumber);

#endif
