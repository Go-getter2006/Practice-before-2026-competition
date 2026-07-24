#include "UART.h"
#include "ti_msp_dl_config.h"

/*******************************************************************************
 * 名    称： UART_SendByteBlocking
 * 功    能：通过指定 UART 外设阻塞发送单个字节。
 * 参    数：uart：UART 外设寄存器地址；byte：待发送字节。
 * 出    口：无返回值。
 * 说    明：该函数仅供本文件内部复用，用于隔离 TI DriverLib 调用。
 *******************************************************************************/
static void UART_SendByteBlocking(UART_Regs *uart, uint8_t byte)
{
    DL_UART_Main_transmitData(uart, byte);
    while (DL_UART_Main_isBusy(uart)) { }
}

/*******************************************************************************
 * 名    称： UART0_Init
 * 功    能：初始化 UART0 接收中断，用作调试串口的接收入口。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_Init(void)
{
    DL_UART_Main_enableInterrupt(UART_0_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
}

/*******************************************************************************
 * 名    称： UART0_SendByte
 * 功    能：通过 UART0 发送 1 个字节，并等待发送完成。
 * 参    数：byte：需要发送的字节。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_SendByte(uint8_t byte)
{
    UART_SendByteBlocking(UART_0_INST, byte);
}

/*******************************************************************************
 * 名    称： UART0_SendString
 * 功    能：通过 UART0 连续发送字符串，常用于串口调试输出。
 * 参    数：str：以 '\0' 结尾的字符串指针。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_SendString(const char *str)
{
    while (*str) {
        UART0_SendByte((uint8_t)*str++);
    }
}

/*******************************************************************************
 * 名    称： UART0_SendInt32
 * 功    能：将有符号 32 位整数转换为十进制文本并通过 UART0 发送。
 * 参    数：value：需要发送的整数。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_SendInt32(int32_t value)
{
    char buf[11];
    uint32_t num;
    uint32_t index = 0U;

    if (value < 0) {
        UART0_SendByte('-');
        num = (uint32_t)(-(value + 1)) + 1U;
    } else {
        num = (uint32_t)value;
    }

    do {
        buf[index++] = (char)('0' + (num % 10U));
        num /= 10U;
    } while (num != 0U);

    while (index > 0U) {
        UART0_SendByte((uint8_t)buf[--index]);
    }
}

/*******************************************************************************
 * 名    称： UART0_SendFloat2
 * 功    能：按两位小数格式发送浮点数，避免嵌入式库浮点 sprintf 占用大量栈。
 * 参    数：value：需要发送的浮点数。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_SendFloat2(float value)
{
    uint32_t scaled;
    uint32_t integer;
    uint32_t fraction;

    if (value < 0.0f) {
        UART0_SendByte('-');
        value = -value;
    }

    scaled = (uint32_t)(value * 100.0f + 0.5f);
    integer = scaled / 100U;
    fraction = scaled % 100U;

    UART0_SendInt32((int32_t)integer);
    UART0_SendByte('.');

    if (fraction < 10U) {
        UART0_SendByte('0');
    }
    UART0_SendInt32((int32_t)fraction);
}

/*******************************************************************************
 * 名    称： UART_Machine_SendByte
 * 功    能：通过 UART_Machine 阻塞发送 1 个字节。
 * 参    数：byte：待发送字节。
 * 出    口：无返回值。
 *******************************************************************************/
void UART_Machine_SendByte(uint8_t byte)
{
    UART_SendByteBlocking(UART_Machine_INST, byte);
}

/*******************************************************************************
 * 名    称： UART_Machine_SendData
 * 功    能：通过 UART_Machine 依次发送二进制缓冲区中的全部数据。
 * 参    数：data：待发送数据缓冲区；length：待发送字节数。
 * 出    口：无返回值。
 * 说    明：本接口用于设备二进制协议，不经过 printf 或字符流重定向。
 *******************************************************************************/
void UART_Machine_SendData(const uint8_t *data, uint32_t length)
{
    uint32_t index;

    if (data == (const uint8_t *)0) {
        return;
    }

    for (index = 0U; index < length; index++) {
        UART_Machine_SendByte(data[index]);
    }
}

/*******************************************************************************
 * 名    称： UART_Bluetooth_SendData
 * 功    能：通过 UART_Bluetooth 阻塞发送一段二进制数据。
 * 参    数：data：待发送数据缓冲区；length：待发送字节数。
 * 出    口：无返回值。
 ******************************************************************************/
void UART_Bluetooth_SendData(const uint8_t *data, uint32_t length)
{
    uint32_t index;

    if (data == (const uint8_t *)0) {
        return;
    }

    for (index = 0U; index < length; index++) {
        UART_SendByteBlocking(UART_Bluetooth_INST, data[index]);
    }
}

/*******************************************************************************
 * 名    称： UART0_IRQHandler
 * 功    能：处理 UART0 接收中断，当前仅清除接收到的数据。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void UART0_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            DL_UART_Main_receiveData(UART_0_INST);
            break;
        default:
            break;
    }
}
