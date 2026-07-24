/**
  ******************************************************************************
  * @file    Bluetooth.c
  * @brief   USART6 蓝牙三字节控制帧解析
  ******************************************************************************
  * @details
  * 支持的控制帧：
  * - FF AA FE：采集电机绝对角度并反向对准雷达目标角度；
  * - FF AB FE：电机返回单圈编码器绝对零位。
  *
  * 中断回调仅解析并缓存命令，电机控制由主循环执行。
  ******************************************************************************
  */

#include "Bluetooth.h"

#define BLUETOOTH_COMMAND_QUEUE_SIZE 8U

typedef enum
{
    BLUETOOTH_RX_WAIT_HEAD = 0U,
    BLUETOOTH_RX_WAIT_COMMAND,
    BLUETOOTH_RX_WAIT_TAIL
} Bluetooth_RxState;

static Bluetooth_RxState bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
static Bluetooth_Command bluetooth_rx_command = BLUETOOTH_COMMAND_NONE;
static volatile Bluetooth_Command bluetooth_command_queue[BLUETOOTH_COMMAND_QUEUE_SIZE];
static volatile uint8_t bluetooth_queue_write_index = 0U;
static volatile uint8_t bluetooth_queue_read_index = 0U;

/**
  * @brief 将一条有效命令写入单生产者、单消费者环形队列。
  * @param command 需要写入的命令。
  * @note  队列满时丢弃最新命令，避免覆盖尚未执行的命令。
  */
static void Bluetooth_QueuePush(Bluetooth_Command command)
{
    uint8_t next_index = (uint8_t)((bluetooth_queue_write_index + 1U) %
                                   BLUETOOTH_COMMAND_QUEUE_SIZE);

    if (next_index == bluetooth_queue_read_index)
    {
        return;
    }

    bluetooth_command_queue[bluetooth_queue_write_index] = command;
    bluetooth_queue_write_index = next_index;
}

void Bluetooth_Init(void)
{
    bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
    bluetooth_rx_command = BLUETOOTH_COMMAND_NONE;
    bluetooth_queue_write_index = 0U;
    bluetooth_queue_read_index = 0U;
}

uint8_t Bluetooth_RxByteCallback(uint8_t data)
{
    switch (bluetooth_rx_state)
    {
        case BLUETOOTH_RX_WAIT_HEAD:
            if (data == BLUETOOTH_FRAME_HEAD)
            {
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_COMMAND;
                return 1U;
            }
            return 0U;

        case BLUETOOTH_RX_WAIT_COMMAND:
            if (data == BLUETOOTH_FRAME_HEAD)
            {
                /* 连续帧头：将最新的 0xFF 作为新帧起点。 */
                return 1U;
            }

            if ((data == BLUETOOTH_CMD_CODE_ALIGN_LIDAR) ||
                (data == BLUETOOTH_CMD_CODE_RETURN_ZERO))
            {
                bluetooth_rx_command = (Bluetooth_Command)data;
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_TAIL;
            }
            else
            {
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
            }
            return 1U;

        case BLUETOOTH_RX_WAIT_TAIL:
            if (data == BLUETOOTH_FRAME_TAIL)
            {
                Bluetooth_QueuePush(bluetooth_rx_command);
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
            }
            else if (data == BLUETOOTH_FRAME_HEAD)
            {
                /* 尾字节错误但遇到新帧头，立即重新同步。 */
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_COMMAND;
            }
            else
            {
                bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
            }

            bluetooth_rx_command = BLUETOOTH_COMMAND_NONE;
            return 1U;

        default:
            bluetooth_rx_state = BLUETOOTH_RX_WAIT_HEAD;
            bluetooth_rx_command = BLUETOOTH_COMMAND_NONE;
            return 0U;
    }
}

Bluetooth_Command Bluetooth_GetCommand(void)
{
    Bluetooth_Command command;

    if (bluetooth_queue_read_index == bluetooth_queue_write_index)
    {
        return BLUETOOTH_COMMAND_NONE;
    }

    command = bluetooth_command_queue[bluetooth_queue_read_index];
    bluetooth_queue_read_index = (uint8_t)((bluetooth_queue_read_index + 1U) %
                                           BLUETOOTH_COMMAND_QUEUE_SIZE);

    return command;
}
