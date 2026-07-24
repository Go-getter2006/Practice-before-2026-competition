#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 蓝牙控制帧格式：帧头 + 命令 + 帧尾。 */
#define BLUETOOTH_FRAME_HEAD            0xFFU
#define BLUETOOTH_FRAME_TAIL            0xFEU
#define BLUETOOTH_CMD_CODE_ALIGN_LIDAR  0xAAU
#define BLUETOOTH_CMD_CODE_RETURN_ZERO  0xABU

/** 主循环可读取的蓝牙控制命令。 */
typedef enum
{
    BLUETOOTH_COMMAND_NONE = 0x00U,
    BLUETOOTH_COMMAND_ALIGN_LIDAR = BLUETOOTH_CMD_CODE_ALIGN_LIDAR,
    BLUETOOTH_COMMAND_RETURN_ZERO = BLUETOOTH_CMD_CODE_RETURN_ZERO
} Bluetooth_Command;

/**
  * @brief 初始化蓝牙帧解析器和命令队列。
  */
void Bluetooth_Init(void);

/**
  * @brief  在 USART6 接收中断中输入一个字节。
  * @param  data USART6 接收到的字节。
  * @retval 1 表示该字节属于蓝牙二进制帧；0 表示普通数据。
  */
uint8_t Bluetooth_RxByteCallback(uint8_t data);

/**
  * @brief  从命令队列读取一条命令。
  * @retval 收到的命令；队列为空时返回 BLUETOOTH_COMMAND_NONE。
  */
Bluetooth_Command Bluetooth_GetCommand(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_BLUETOOTH_H_ */
