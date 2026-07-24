#include "serial.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>   // 可选，用于 memcpy，但本实现未使用
#include "JY901.h"
#include "Bluetooth.h"
#include "Lidar.h"
#include "machine.h"

// 外部声明 CubeMX 生成的 UART 句柄
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;           // USART2 持续接收MS42DDC反馈
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;           // USART6 仅接收蓝牙二进制命令
extern uint8_t uart5_rx_byte;       // JY901.c定义的UART5接收字节

// 全局变量定义
uint8_t Serial_TxPacket[SERIAL_MAX_DATA_LEN];
uint8_t Serial_RxPacket[SERIAL_MAX_DATA_LEN];
volatile uint8_t Serial_RxFlag = 0;
volatile uint8_t move = 0;                   // 接收到的移动指令数据

uint8_t uart6_rx_byte = 0;          
char uart6_line[128] = {0};
volatile uint8_t uart6_line_ready = 0;

/**
  * @brief 初始化旧串口数据区。
  * @note  USART1 已专用于 N10 雷达，接收中断由 Lidar_Init() 启动。
  */
void Serial_Init(void)
{
    Serial_RxFlag = 0;
    move = 0;
}

/**
  * @brief  通过 USART1 阻塞发送一个字节。
  * @param  Byte 待发送的字节。
  */
void Serial_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart1, &Byte, 1, HAL_MAX_DELAY);
}

/**
  * @brief  通过 USART1 阻塞发送一个字节数组。
  * @param  Array  待发送数组的首地址。
  * @param  Length 待发送数组的长度。
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    HAL_UART_Transmit(&huart1, Array, Length, HAL_MAX_DELAY);
}

/**
  * @brief  通过 USART1 阻塞发送以 '\0' 结尾的字符串。
  * @param  String 待发送字符串的首地址。
  */
void Serial_SendString(char *String)
{
    uint16_t len = 0;
    while (String[len] != '\0') len++;
    if (len > 0) {
        HAL_UART_Transmit(&huart1, (uint8_t*)String, len, HAL_MAX_DELAY);
    }
}

/**
  * @brief  计算无符号整数的幂。
  * @param  X 底数。
  * @param  Y 指数。
  * @retval X 的 Y 次方。
  */
static uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--) Result *= X;
    return Result;
}

/**
  * @brief  通过 USART1 发送指定宽度的无符号十进制数字。
  * @param  Number 待发送数字，范围为 0～4294967295。
  * @param  Length 输出位数，范围为 0～10，不足位使用前导零补齐。
  */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++) {
        uint8_t digit = (Number / Serial_Pow(10, Length - i - 1)) % 10;
        Serial_SendByte(digit + '0');
    }
}

/**
  * @brief  将标准输出的单个字符重定向到 USART1。
  * @param  ch 待输出字符。
  * @param  f  标准库传入的文件指针，本函数未使用。
  * @retval 已发送的字符。
  */
int fputc(int ch, FILE *f)
{
    Serial_SendByte((uint8_t)ch);
    return ch;
}

/**
  * @brief  格式化字符串并通过 USART1 阻塞发送。
  * @param  format printf 风格的格式字符串。
  * @param  ...    与格式字符串对应的可变参数。
  */
void Serial_Printf(char *format, ...)
{
    char String[100];
    va_list arg;
    va_start(arg, format);
    vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);
    Serial_SendString(String);
}

/**
  * @brief  按固定帧格式通过 USART1 发送不定长数据包。
  * @param  dataLength Serial_TxPacket 中的有效数据长度。
  * @note   数据长度不能超过 SERIAL_MAX_DATA_LEN。
  */
void Serial_SendPacket(uint8_t dataLength)
{
    uint8_t i;
    uint8_t checksum = 0;

    if (dataLength > SERIAL_MAX_DATA_LEN) {
        dataLength = SERIAL_MAX_DATA_LEN;   // 防止越界，或可返回错误
    }

    Serial_SendByte(0xFF);                   // 帧头
    Serial_SendByte(dataLength);              // 长度

    for (i = 0; i < dataLength; i++) {
        Serial_SendByte(Serial_TxPacket[i]);
        checksum ^= Serial_TxPacket[i];       // 异或校验
    }

    Serial_SendByte(checksum);                 // 校验字节
    Serial_SendByte(0xFE);                      // 帧尾
}

/**
  * @brief  读取并清除串口数据包接收完成标志。
  * @retval 1：收到完整数据包；0：没有新的完整数据包。
  */
uint8_t Serial_GetRxFlag(void)
{
    if (Serial_RxFlag == 1) {
        Serial_RxFlag = 0;
        return 1;
    }
    return 0;
}


void USART6_RX_Init(void)
{
    /* 清除可能残留的接收状态和错误标志，再启动单字节中断接收。 */
    (void)HAL_UART_AbortReceive(&huart6);
    __HAL_UART_CLEAR_PEFLAG(&huart6);
    huart6.ErrorCode = HAL_UART_ERROR_NONE;
    (void)HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1U);
}


/**
  * @brief  分发各串口的单字节接收完成事件并重新启动接收。
  * @param  huart 触发回调的串口句柄指针。
  * @note USART1用于雷达，USART2用于电机反馈，UART5用于IMU，USART6用于蓝牙。
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        /* USART1 专用于接收 N10 雷达数据，不再运行旧的 FF/FE 数据包状态机。 */
        Lidar_UART_RxCpltCallback();
    }
    else if (huart->Instance == USART2) {
        /* USART2从上电开始持续接收MS42DDC反馈，兼容20 Hz和请求方式。 */
        Machine_FeedbackRxByteCallback();
    }
    else if (huart->Instance == UART5) {
        JY901_RxCallback(uart5_rx_byte);
    }
    else if (huart->Instance == USART6) {
#if 0
        /* 备用功能：原 USART6 字符串调参行缓冲，当前暂时停用。 */
        static uint8_t idx = 0;

        if (!Bluetooth_RxByteCallback(uart6_rx_byte)) {
            if (uart6_rx_byte == ']') {
                if (idx < sizeof(uart6_line) - 1U) {
                    uart6_line[idx++] = ']';
                }
                uart6_line[idx] = '\0';
                uart6_line_ready = 1;
                idx = 0;
            } else if (idx < sizeof(uart6_line) - 1U) {
                uart6_line[idx++] = (char)uart6_rx_byte;
            }
        }
#else
        /* 当前仅解析 FF AA FE 和 FF AB FE，其他字节直接忽略。 */
        (void)Bluetooth_RxByteCallback(uart6_rx_byte);
#endif
        HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Lidar_UART_ErrorCallback();
    }
    else if (huart->Instance == USART2)
    {
        Machine_FeedbackRxErrorCallback();
    }
    else if (huart->Instance == USART6)
    {
        /*
         * USART6 出现溢出、帧错误或噪声错误时，放弃当前半帧并立即恢复接收。
         * 否则一次串口错误就会让 FF AA FE 永远无法再进入接收回调。
         */
        (void)HAL_UART_AbortReceive(&huart6);
        __HAL_UART_CLEAR_PEFLAG(&huart6);
        huart6.ErrorCode = HAL_UART_ERROR_NONE;
        (void)Bluetooth_RxByteCallback(0U);
        (void)HAL_UART_Receive_IT(&huart6, &uart6_rx_byte, 1U);
    }
}

/*
   USART6 slider 协议接收参考（原 main.c 代码）：
   --- 初始化调用 ---
   USART6_RX_Init();
   --- 主循环中解析 ---
   if (uart6_line_ready) {
     uart6_line_ready = 0;
     char *tag = strtok(uart6_line, "[], ");
     if (tag && strcmp(tag, "slider") == 0) {
       char *name = strtok(NULL, "[], ");
       char *val  = strtok(NULL, "[], ");
       if (name && val) {
         float f = atof(val);
         if      (strcmp(name, "Kp1") == 0) left_pid.kp = f;
         else if (strcmp(name, "Ki1") == 0) left_pid.ki = f;
         else if (strcmp(name, "Kd1") == 0) left_pid.kd = f;
         else if (strcmp(name, "Kp2") == 0) right_pid.kp = f;
         else if (strcmp(name, "Ki2") == 0) right_pid.ki = f;
         else if (strcmp(name, "Kd2") == 0) right_pid.kd = f;
         else if (strcmp(name, "target") == 0) {
           left_target = f;  right_target = f;
         }
       }
     }
   }
*/
