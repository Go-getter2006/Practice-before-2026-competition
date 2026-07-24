#ifndef MAXICAM_H
#define MAXICAM_H

#include <stdbool.h>
#include <stdint.h>

/* Maxicam 通信帧中的固定帧头和帧尾。 */
#define MAXICAM_FRAME_HEADER          (0xFFU)
#define MAXICAM_FRAME_TAIL            (0xFEU)

/* 数据包个数占一个字节，因此单帧最多携带二百五十五个数据字节。 */
#define MAXICAM_DATA_MAX_LENGTH       (255U)

/**
 * @brief Maxicam 完整数据帧，length 表示 data 中的有效字节数。
 */
typedef struct {
    uint8_t length;
    uint8_t data[MAXICAM_DATA_MAX_LENGTH];
} Maxicam_Frame_t;

/*******************************************************************************
 * 名    称：Maxicam_Init
 * 功    能：复位 Maxicam 协议解析状态并开启串口接收中断。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：调用本函数前必须先执行 SYSCFG_DL_init()。
 *******************************************************************************/
void Maxicam_Init(void);

/*******************************************************************************
 * 名    称：Maxicam_RxCallback
 * 功    能：向 Maxicam 协议解析器送入一个接收到的字节。
 * 参    数：data：串口接收到的单字节数据。
 * 出    口：无返回值。
 * 说    明：正常使用时由串口中断自动调用，也可用于协议测试。
 *******************************************************************************/
void Maxicam_RxCallback(uint8_t data);

/*******************************************************************************
 * 名    称：Maxicam_ReadFrame
 * 功    能：读取自上次调用后收到的最新一帧有效数据。
 * 参    数：frame：用于保存数据包个数和数据包内容的结构体指针。
 * 出    口：存在未读取的新帧时返回真，否则返回假。
 * 说    明：多帧数据未及时读取时只保留最新一帧，旧帧会被直接覆盖。
 *******************************************************************************/
bool Maxicam_ReadFrame(Maxicam_Frame_t *frame);

#endif
