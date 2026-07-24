#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "main.h"

// 最大数据长度（可根据需要调整，不超过255）
#define SERIAL_MAX_DATA_LEN     255

// 全局变量声明
extern uint8_t Serial_TxPacket[SERIAL_MAX_DATA_LEN];   // 发送数据缓冲区
extern uint8_t Serial_RxPacket[SERIAL_MAX_DATA_LEN];   // 接收数据缓冲区
extern volatile uint8_t Serial_RxFlag;                 // 接收完成标志（1：已收到完整包）
extern volatile uint8_t move;                          // 接收到的移动指令数据（1字节）
extern uint8_t uart6_rx_byte;                  
extern char uart6_line[128];
extern volatile uint8_t uart6_line_ready;

// 函数声明
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);

// 发送不定长数据包（dataLength：数据字节数，需 ≤ SERIAL_MAX_DATA_LEN）
void Serial_SendPacket(uint8_t dataLength);

// 获取接收完成标志（读取后自动清零）
uint8_t Serial_GetRxFlag(void);

void USART6_RX_Init(void);         

// HAL库回调函数声明
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif
