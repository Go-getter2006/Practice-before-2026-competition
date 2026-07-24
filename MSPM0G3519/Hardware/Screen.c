#include "Screen.h"
#include "ti_msp_dl_config.h"

/* 接收状态分别表示等待帧头、等待任务号和等待帧尾。 */
typedef enum {
    SCREEN_RX_WAIT_HEADER = 0,
    SCREEN_RX_WAIT_TASK,
    SCREEN_RX_WAIT_TAIL
} Screen_RxState;

/* 队列长度必须大于当前可能连续接收的任务数量。 */
#define SCREEN_TASK_QUEUE_SIZE    (8U)

static volatile Screen_RxState g_screenRxState = SCREEN_RX_WAIT_HEADER;
static volatile uint8_t g_screenCurrentTask = 0U;
static volatile uint8_t g_screenTaskQueue[SCREEN_TASK_QUEUE_SIZE];
static volatile uint8_t g_screenQueueWrite = 0U;
static volatile uint8_t g_screenQueueRead = 0U;

/*******************************************************************************
 * 名    称：Screen_IsTaskValid
 * 功    能：判断任务号是否属于当前支持的任务范围。
 * 参    数：taskNumber：待判断的任务号。
 * 出    口：任务号有效返回真，否则返回假。
 *******************************************************************************/
static bool Screen_IsTaskValid(uint8_t taskNumber)
{
    return (taskNumber >= SCREEN_TASK_1) && (taskNumber <= SCREEN_TASK_3);
}

/*******************************************************************************
 * 名    称：Screen_PushTask
 * 功    能：把完整通信帧中的任务号放入待执行队列。
 * 参    数：taskNumber：需要进入队列的任务号。
 * 出    口：无返回值。
 * 说    明：队列已满时丢弃新任务，避免覆盖尚未执行的任务。
 *******************************************************************************/
static void Screen_PushTask(uint8_t taskNumber)
{
    uint8_t nextWrite = (uint8_t)((g_screenQueueWrite + 1U) % SCREEN_TASK_QUEUE_SIZE);

    if (nextWrite == g_screenQueueRead) {
        return;
    }

    g_screenTaskQueue[g_screenQueueWrite] = taskNumber;
    g_screenQueueWrite = nextWrite;
}

/*******************************************************************************
 * 名    称：Screen_ParseByte
 * 功    能：按“帧头、任务号、帧尾”的顺序解析一个接收字节。
 * 参    数：data：UART_Screen 接收到的字节。
 * 出    口：无返回值。
 * 说    明：任意位置再次收到帧头时，立即从新帧开始解析。
 *******************************************************************************/
static void Screen_ParseByte(uint8_t data)
{
    if (data == SCREEN_FRAME_HEADER) {
        g_screenRxState = SCREEN_RX_WAIT_TASK;
        return;
    }

    switch (g_screenRxState) {
        case SCREEN_RX_WAIT_TASK:
            if (Screen_IsTaskValid(data)) {
                g_screenCurrentTask = data;
                g_screenRxState = SCREEN_RX_WAIT_TAIL;
            } else {
                g_screenRxState = SCREEN_RX_WAIT_HEADER;
            }
            break;

        case SCREEN_RX_WAIT_TAIL:
            if (data == SCREEN_FRAME_TAIL) {
                Screen_PushTask(g_screenCurrentTask);
            }
            g_screenRxState = SCREEN_RX_WAIT_HEADER;
            break;

        case SCREEN_RX_WAIT_HEADER:
        default:
            break;
    }
}

/*******************************************************************************
 * 名    称：Screen_Init
 * 功    能：初始化串口屏接收状态，并开启 UART_Screen 接收中断。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void Screen_Init(void)
{
    g_screenRxState = SCREEN_RX_WAIT_HEADER;
    g_screenCurrentTask = 0U;
    g_screenQueueWrite = 0U;
    g_screenQueueRead = 0U;

    DL_UART_Main_enableInterrupt(UART_Screen_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_Screen_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_Screen_INST_INT_IRQN);
}

/*******************************************************************************
 * 名    称：Screen_ReadTask
 * 功    能：从串口屏任务队列中读取一个待执行任务号。
 * 参    数：taskNumber：用于保存任务号的指针。
 * 出    口：读取成功返回真，当前没有任务或指针为空时返回假。
 *******************************************************************************/
bool Screen_ReadTask(uint8_t *taskNumber)
{
    if ((taskNumber == (uint8_t *)0) || (g_screenQueueRead == g_screenQueueWrite)) {
        return false;
    }

    *taskNumber = g_screenTaskQueue[g_screenQueueRead];
    g_screenQueueRead = (uint8_t)((g_screenQueueRead + 1U) % SCREEN_TASK_QUEUE_SIZE);
    return true;
}

/*******************************************************************************
 * 名    称：UART_Screen_INST_IRQHandler
 * 功    能：接收串口屏数据，并将每个字节交给协议解析器。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：UART_Screen_INST_IRQHandler 会由系统配置映射到实际中断函数名。
 *******************************************************************************/
void UART_Screen_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_Screen_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            while (!DL_UART_Main_isRXFIFOEmpty(UART_Screen_INST)) {
                Screen_ParseByte(DL_UART_Main_receiveData(UART_Screen_INST));
            }
            break;

        default:
            break;
    }
}
