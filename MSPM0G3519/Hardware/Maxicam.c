#include "Maxicam.h"
#include "ti_msp_dl_config.h"

/* 接收状态对应帧头、数据包个数、数据包、校验位和帧尾五个阶段。 */
typedef enum {
    MAXICAM_RX_WAIT_HEADER = 0,
    MAXICAM_RX_WAIT_LENGTH,
    MAXICAM_RX_RECEIVE_DATA,
    MAXICAM_RX_WAIT_BCC,
    MAXICAM_RX_WAIT_TAIL
} Maxicam_RxState_t;

/* 当前正在接收的帧及其协议解析状态。 */
static Maxicam_RxState_t s_rxState = MAXICAM_RX_WAIT_HEADER;
static uint8_t s_rxData[MAXICAM_DATA_MAX_LENGTH];
static uint8_t s_rxLength = 0U;
static uint8_t s_rxIndex = 0U;
static uint8_t s_rxBcc = 0U;
static uint8_t s_rxReceivedBcc = 0U;

/* 始终只保存最近一次通过校验的完整数据帧。 */
static volatile Maxicam_Frame_t s_latestFrame;
static volatile bool s_frameReady = false;

/*******************************************************************************
 * 名    称：Maxicam_ResetParser
 * 功    能：丢弃当前未完成帧并恢复到等待帧头状态。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
static void Maxicam_ResetParser(void)
{
    s_rxState = MAXICAM_RX_WAIT_HEADER;
    s_rxLength = 0U;
    s_rxIndex = 0U;
    s_rxBcc = 0U;
    s_rxReceivedBcc = 0U;
}

/*******************************************************************************
 * 名    称：Maxicam_StartFrame
 * 功    能：记录帧头并开始接收一帧新的 Maxicam 数据。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：BCC 从帧头开始计算，因此初始值为固定帧头 0xFF。
 *******************************************************************************/
static void Maxicam_StartFrame(void)
{
    s_rxState = MAXICAM_RX_WAIT_LENGTH;
    s_rxLength = 0U;
    s_rxIndex = 0U;
    s_rxBcc = MAXICAM_FRAME_HEADER;
    s_rxReceivedBcc = 0U;
}

/*******************************************************************************
 * 名    称：Maxicam_UpdateLatestFrame
 * 功    能：使用当前通过校验的数据覆盖上一帧有效数据。
 * 参    数：无，数据取自本文件内部接收缓冲区。
 * 出    口：无返回值。
 * 说    明：无论上一帧是否已经读取，新帧都会直接覆盖旧帧。
 *******************************************************************************/
static void Maxicam_UpdateLatestFrame(void)
{
    uint8_t index;

    s_latestFrame.length = s_rxLength;
    for (index = 0U; index < s_rxLength; index++) {
        s_latestFrame.data[index] = s_rxData[index];
    }

    s_frameReady = true;
}

/*******************************************************************************
 * 名    称：Maxicam_Init
 * 功    能：复位 Maxicam 协议解析状态并开启串口接收中断。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void Maxicam_Init(void)
{
    uint8_t unusedData;

    NVIC_DisableIRQ(UART_Maixcam_INST_INT_IRQN);
    Maxicam_ResetParser();
    s_latestFrame.length = 0U;
    s_frameReady = false;

    while (DL_UART_Main_receiveDataCheck(UART_Maixcam_INST, &unusedData)) {
        /* 清空初始化前已经进入接收先进先出缓冲区的残留数据。 */
    }

    DL_UART_Main_clearInterruptStatus(UART_Maixcam_INST,
                                      DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_enableInterrupt(UART_Maixcam_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_Maixcam_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Maixcam_INST_INT_IRQN);
}

/*******************************************************************************
 * 名    称：Maxicam_RxCallback
 * 功    能：按照“FF、数据包个数、数据包、BCC、FE”逐字节解析通信帧。
 * 参    数：data：串口接收到的单字节数据。
 * 出    口：无返回值。
 * 说    明：BCC 为帧头、数据包个数和全部数据字节的异或结果。
 *******************************************************************************/
void Maxicam_RxCallback(uint8_t data)
{
    switch (s_rxState) {
        case MAXICAM_RX_WAIT_HEADER:
            if (data == MAXICAM_FRAME_HEADER) {
                Maxicam_StartFrame();
            }
            break;

        case MAXICAM_RX_WAIT_LENGTH:
            s_rxLength = data;
            s_rxIndex = 0U;
            s_rxBcc ^= data;
            if (s_rxLength == 0U) {
                s_rxState = MAXICAM_RX_WAIT_BCC;
            } else {
                s_rxState = MAXICAM_RX_RECEIVE_DATA;
            }
            break;

        case MAXICAM_RX_RECEIVE_DATA:
            s_rxData[s_rxIndex] = data;
            s_rxIndex++;
            s_rxBcc ^= data;
            if (s_rxIndex >= s_rxLength) {
                s_rxState = MAXICAM_RX_WAIT_BCC;
            }
            break;

        case MAXICAM_RX_WAIT_BCC:
            s_rxReceivedBcc = data;
            s_rxState = MAXICAM_RX_WAIT_TAIL;
            break;

        case MAXICAM_RX_WAIT_TAIL:
            if ((data == MAXICAM_FRAME_TAIL) &&
                (s_rxReceivedBcc == s_rxBcc)) {
                Maxicam_UpdateLatestFrame();
            }

            if (data == MAXICAM_FRAME_HEADER) {
                Maxicam_StartFrame();
            } else {
                Maxicam_ResetParser();
            }
            break;

        default:
            Maxicam_ResetParser();
            break;
    }
}

/*******************************************************************************
 * 名    称：Maxicam_ReadFrame
 * 功    能：读取自上次调用后收到的最新一帧有效数据。
 * 参    数：frame：用于保存数据包个数和数据包内容的结构体指针。
 * 出    口：存在未读取的新帧时返回真，否则返回假。
 * 说    明：复制期间暂时关闭 Maxicam 串口中断，避免新帧覆盖造成数据不一致。
 *******************************************************************************/
bool Maxicam_ReadFrame(Maxicam_Frame_t *frame)
{
    uint8_t index;

    if (frame == (Maxicam_Frame_t *)0) {
        return false;
    }

    NVIC_DisableIRQ(UART_Maixcam_INST_INT_IRQN);
    if (s_frameReady == false) {
        NVIC_EnableIRQ(UART_Maixcam_INST_INT_IRQN);
        return false;
    }

    frame->length = s_latestFrame.length;
    for (index = 0U; index < frame->length; index++) {
        frame->data[index] = s_latestFrame.data[index];
    }

    s_frameReady = false;
    NVIC_EnableIRQ(UART_Maixcam_INST_INT_IRQN);
    return true;
}

/*******************************************************************************
 * 名    称：UART_Maixcam_INST_IRQHandler
 * 功    能：处理 Maxicam 串口接收中断并把接收字节送入协议解析器。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：该名称由系统配置映射为实际使用的 UART5_IRQHandler。
 *******************************************************************************/
void UART_Maixcam_INST_IRQHandler(void)
{
    uint8_t receivedData;

    while (DL_UART_Main_receiveDataCheck(UART_Maixcam_INST, &receivedData)) {
        Maxicam_RxCallback(receivedData);
    }
}
