#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint32_t nowtime;

/*******************************************************************************
 * 名    称： Timer_Init
 * 功    能：初始化 TIMER_Clock 中断。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void Timer_Init(void);

#endif
