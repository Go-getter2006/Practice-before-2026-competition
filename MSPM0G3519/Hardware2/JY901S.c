#include "JY901S.h"
#include "Delay.h"
#include "ti_msp_dl_config.h"

/* JY901S 串口数据帧固定为 11 字节。 */
#define JY901S_FRAME_LENGTH       (11U)

/* 角度数据帧的帧头与数据类型。 */
#define JY901S_FRAME_HEADER       (0x55U)
#define JY901S_ANGLE_FRAME_TYPE   (0x53U)

/* 原始角度值换算为角度值时使用的比例系数。 */
#define JY901S_ANGLE_SCALE        (180.0f / 32768.0f)

volatile JY901S_Angles_t JY901S_Angles = {0.0f, 0.0f, 0.0f};
volatile float yaw = 0.0f;

/* 协议解析缓冲区及当前写入位置。 */
static uint8_t jy901s_frame[JY901S_FRAME_LENGTH];
static uint8_t jy901s_index = 0U;

/* 有效帧标志在首帧校验成功后置位。 */
static volatile uint8_t jy901s_data_valid = 0U;

/*******************************************************************************
 * 名    称：JY901S_SendData
 * 功    能：通过 UART_JY901S 阻塞发送一段命令数据。
 * 参    数：data：待发送数据；length：待发送字节数。
 * 出    口：无返回值。
 *******************************************************************************/
static void JY901S_SendData(const uint8_t *data, uint8_t length)
{
    uint8_t index;

    if (data == (const uint8_t *)0) {
        return;
    }

    for (index = 0U; index < length; index++) {
        DL_UART_Main_transmitDataBlocking(UART_JY901S_INST, data[index]);
    }
}

/*******************************************************************************
 * 名    称：JY901S_ParseAngleFrame
 * 功    能：校验并解析完整的 0x53 角度数据帧。
 * 参    数：无，数据取自本文件内部帧缓冲区。
 * 出    口：无返回值。
 *******************************************************************************/
static void JY901S_ParseAngleFrame(void)
{
    uint8_t index;
    uint8_t checksum = 0U;
    int16_t roll_raw;
    int16_t pitch_raw;
    int16_t yaw_raw;
    float roll_value;
    float pitch_value;
    float yaw_value;

    for (index = 0U; index < (JY901S_FRAME_LENGTH - 1U); index++) {
        checksum = (uint8_t)(checksum + jy901s_frame[index]);
    }

    if (checksum != jy901s_frame[JY901S_FRAME_LENGTH - 1U]) {
        return;
    }

    roll_raw = (int16_t)(((uint16_t)jy901s_frame[3] << 8U) |
                         (uint16_t)jy901s_frame[2]);
    pitch_raw = (int16_t)(((uint16_t)jy901s_frame[5] << 8U) |
                          (uint16_t)jy901s_frame[4]);
    yaw_raw = (int16_t)(((uint16_t)jy901s_frame[7] << 8U) |
                        (uint16_t)jy901s_frame[6]);

    roll_value = (float)roll_raw * JY901S_ANGLE_SCALE;
    pitch_value = (float)pitch_raw * JY901S_ANGLE_SCALE;
    yaw_value = (float)yaw_raw * JY901S_ANGLE_SCALE;

    JY901S_Angles.roll = roll_value;
    JY901S_Angles.pitch = pitch_value;
    JY901S_Angles.yaw = yaw_value;
    yaw = yaw_value;
    jy901s_data_valid = 1U;
}

/*******************************************************************************
 * 名    称：JY901S_Init
 * 功    能：复位协议解析状态，清空残留接收数据并开启 JY901S 串口中断。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void JY901S_Init(void)
{
    uint8_t unused_data;

    jy901s_index = 0U;
    jy901s_data_valid = 0U;

    while (DL_UART_Main_receiveDataCheck(UART_JY901S_INST, &unused_data)) {
        /* 清空初始化前已经进入接收先进先出缓冲区的残留数据。 */
    }

    DL_UART_Main_clearInterruptStatus(UART_JY901S_INST,
                                      DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_enableInterrupt(UART_JY901S_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_JY901S_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_JY901S_INST_INT_IRQN);
}

/*******************************************************************************
 * 名    称：JY901S_SetZeroRef
 * 功    能：解锁传感器，将当前姿态设置为角度零点并保存配置。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void JY901S_SetZeroRef(void)
{
    static const uint8_t unlock_command[5] = {0xFFU, 0xAAU, 0x69U, 0x88U, 0xB5U};
    static const uint8_t zero_command[5]   = {0xFFU, 0xAAU, 0x01U, 0x08U, 0x00U};
    static const uint8_t save_command[5]   = {0xFFU, 0xAAU, 0x00U, 0x00U, 0x00U};

    Delay_ms(500U);
    JY901S_SendData(unlock_command, 5U);
    Delay_ms(200U);
    JY901S_SendData(zero_command, 5U);
    Delay_ms(2000U);
    JY901S_SendData(save_command, 5U);
    Delay_ms(100U);
}

/*******************************************************************************
 * 名    称：JY901S_RxCallback
 * 功    能：逐字节同步并接收 JY901S 的 0x53 角度数据帧。
 * 参    数：data：串口接收到的单字节数据。
 * 出    口：无返回值。
 *******************************************************************************/
void JY901S_RxCallback(uint8_t data)
{
    if (jy901s_index == 0U) {
        if (data == JY901S_FRAME_HEADER) {
            jy901s_frame[0] = data;
            jy901s_index = 1U;
        }
        return;
    }

    if (jy901s_index == 1U) {
        if (data == JY901S_ANGLE_FRAME_TYPE) {
            jy901s_frame[1] = data;
            jy901s_index = 2U;
        } else if (data == JY901S_FRAME_HEADER) {
            /* 连续出现帧头时保留后一字节，继续等待角度帧类型。 */
            jy901s_frame[0] = data;
        } else {
            jy901s_index = 0U;
        }
        return;
    }

    jy901s_frame[jy901s_index] = data;
    jy901s_index++;

    if (jy901s_index >= JY901S_FRAME_LENGTH) {
        jy901s_index = 0U;
        JY901S_ParseAngleFrame();
    }
}

/*******************************************************************************
 * 名    称：JY901S_GetAngles
 * 功    能：读取最近一次通过校验的横滚角、俯仰角和偏航角。
 * 参    数：angles：用于接收角度数据的结构体指针。
 * 出    口：uint8_t，读取成功返回 1，指针为空返回 0。
 *******************************************************************************/
uint8_t JY901S_GetAngles(JY901S_Angles_t *angles)
{
    if (angles == (JY901S_Angles_t *)0) {
        return 0U;
    }

    angles->roll = JY901S_Angles.roll;
    angles->pitch = JY901S_Angles.pitch;
    angles->yaw = JY901S_Angles.yaw;
    return 1U;
}

/*******************************************************************************
 * 名    称：JY901S_HasValidData
 * 功    能：判断是否已经接收到至少一帧通过校验的角度数据。
 * 参    数：无。
 * 出    口：uint8_t，有有效数据返回 1，否则返回 0。
 *******************************************************************************/
uint8_t JY901S_HasValidData(void)
{
    return jy901s_data_valid;
}

/*******************************************************************************
 * 名    称：UART6_IRQHandler
 * 功    能：处理 UART_JY901S 接收中断，并将接收数据送入协议解析器。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void UART6_IRQHandler(void)
{
    uint8_t received_data;

    while (DL_UART_Main_receiveDataCheck(UART_JY901S_INST, &received_data)) {
        JY901S_RxCallback(received_data);
    }
}
