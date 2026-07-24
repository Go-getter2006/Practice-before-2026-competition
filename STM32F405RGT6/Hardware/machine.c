/**
  ******************************************************************************
  * @file    machine.c
  * @brief   MS42DC 一体式步进电机的 UART2 通信驱动
  ******************************************************************************
  * @details
  * 本文件按照 MS42DC TTL/USB 串口协议组织 11 字节控制帧：
  * 帧头、设备地址、控制模式、方向、细分、位置高/低字节、
  * 速度高/低字节、BCC 校验、帧尾。
  *
  * 所有角度和速度均放大 10 倍后传输，BCC 为前 9 字节的异或值。
  ******************************************************************************
  */

#include "machine.h"
#include "usart.h"
#include <string.h>

/* 私有常量 --------------------------------------------------------------- */
#define MACHINE_UART_TIMEOUT_MS       100U
#define MACHINE_TX_INTERVAL_MS        50U
#define MACHINE_HEAD                  0x7BU
#define MACHINE_TAIL                  0x7DU
#define MACHINE_MODE_FEEDBACK         0x00U
#define MACHINE_MODE_SPEED            0x01U
#define MACHINE_MODE_POSITION         0x02U
#define MACHINE_MODE_TORQUE           0x03U
#define MACHINE_MODE_ABSOLUTE_ANGLE   0x04U
#define MACHINE_ABSOLUTE_ANGLE_MAX_DEG 360.0f

/* USART2反馈的持续中断接收状态，兼容固定20 Hz和请求反馈两种模式。 */
static uint8_t machine_feedback_rx_byte = 0U;
static uint8_t machine_feedback_rx_window[MACHINE_FEEDBACK_FRAME_LEN];
static uint8_t machine_feedback_rx_count = 0U;
static volatile Machine_Feedback machine_feedback_latest = {0};
static volatile uint32_t machine_feedback_sequence = 0U;
static volatile uint8_t machine_feedback_available = 0U;
volatile uint32_t machine_uart2_rx_byte_count = 0U;
volatile uint32_t machine_feedback_valid_frame_count = 0U;
volatile uint32_t machine_feedback_reject_count = 0U;
volatile uint32_t machine_uart2_error_count = 0U;

/* 私有函数声明 ----------------------------------------------------------- */
static float Machine_AbsFloat(float value);
static uint16_t Machine_Scale10Clamp(float value);
static uint8_t Machine_IsFrameParameterValid(uint8_t mode,
                                              Machine_Direction dir,
                                              uint8_t microstep);
static HAL_StatusTypeDef Machine_SendFrame(uint8_t frame[MACHINE_FRAME_LEN]);
static HAL_StatusTypeDef Machine_FeedbackRxArm(void);

/**
  * @brief  计算浮点数绝对值。
  * @param  value 输入值。
  * @retval 输入值的绝对值。
  */
static float Machine_AbsFloat(float value)
{
    return (value < 0.0f) ? -value : value;
}

/**
  * @brief  将角度或速度放大 10 倍并转换为协议中的无符号 16 位数据。
  * @param  value 角度（deg）或速度（rad/s）。
  * @retval 四舍五入后的协议数据；超出范围时限制为 65535。
  */
static uint16_t Machine_Scale10Clamp(float value)
{
    float scaled;

    value = Machine_AbsFloat(value);
    scaled = (value * 10.0f) + 0.5f;

    if (scaled > 65535.0f)
    {
        return 65535U;
    }

    return (uint16_t)scaled;
}

/**
  * @brief  检查控制模式、方向和细分参数是否符合协议。
  * @param  mode      控制模式，范围为 0x00～0x04。
  * @param  dir       电机方向。
  * @param  microstep 细分值；反馈请求模式固定为 0，其他模式支持 2/4/8/16/32。
  * @retval 1 参数有效；0 参数无效。
  */
static uint8_t Machine_IsFrameParameterValid(uint8_t mode,
                                              Machine_Direction dir,
                                              uint8_t microstep)
{
    if ((dir > MACHINE_DIR_CW) || (mode > MACHINE_MODE_ABSOLUTE_ANGLE))
    {
        return 0U;
    }

    if (mode == MACHINE_MODE_FEEDBACK)
    {
        return (microstep == 0U) ? 1U : 0U;
    }

    return Machine_IsMicrostepValid(microstep);
}

/**
  * @brief  通过 UART2 阻塞发送一帧电机控制数据。
  * @param  frame 待发送的 11 字节数据帧。
  * @retval HAL 库返回状态。
  */
static HAL_StatusTypeDef Machine_SendFrame(uint8_t frame[MACHINE_FRAME_LEN])
{
    static uint32_t last_tx_tick = 0U;
    static uint8_t tx_started = 0U;
    uint32_t elapsed;
    HAL_StatusTypeDef status;

    /*
     * 所有 USART2 电机命令共用此出口，统一保证相邻两帧至少间隔 50 ms。
     * 这样周期反馈请求与蓝牙触发命令之间也不会出现间隔不足。
     */
    if (tx_started)
    {
        elapsed = HAL_GetTick() - last_tx_tick;
        if (elapsed < MACHINE_TX_INTERVAL_MS)
        {
            HAL_Delay(MACHINE_TX_INTERVAL_MS - elapsed);
        }
    }

    status = HAL_UART_Transmit(&huart2,
                               frame,
                               MACHINE_FRAME_LEN,
                               MACHINE_UART_TIMEOUT_MS);
    last_tx_tick = HAL_GetTick();
    tx_started = 1U;

    return status;
}

/**
  * @brief  计算 BCC 异或校验值。
  * @param  data 待校验数据首地址。
  * @param  len  待校验数据长度。
  * @retval 所有输入字节的异或结果。
  */
uint8_t Machine_Checksum(const uint8_t *data, uint8_t len)
{
    uint8_t checksum = 0U;
    uint8_t i;

    if (data == 0)
    {
        return 0U;
    }

    for (i = 0U; i < len; i++)
    {
        checksum ^= data[i];
    }

    return checksum;
}

/**
  * @brief  检查步进电机细分值是否受支持。
  * @param  microstep 细分值。
  * @retval 1 支持；0 不支持。
  */
uint8_t Machine_IsMicrostepValid(uint8_t microstep)
{
    return (microstep == 2U) ||
           (microstep == 4U) ||
           (microstep == 8U) ||
           (microstep == 16U) ||
           (microstep == 32U);
}

/**
  * @brief  按照 MS42DC 串口协议构造 11 字节数据帧。
  * @param  frame        输出帧缓冲区。
  * @param  id           电机设备地址，出厂默认值为 0x01。
  * @param  mode         控制模式：0x00～0x04。
  * @param  dir          转动方向：0 逆时针，1 顺时针。
  * @param  microstep    细分值；反馈请求模式传 0，其他模式传 2/4/8/16/32。
  * @param  position_x10 角度放大 10 倍后的数据。
  * @param  speed_x10    速度放大 10 倍后的数据。
  * @retval 1 构帧成功；0 参数错误。
  */
uint8_t Machine_BuildFrame(uint8_t frame[MACHINE_FRAME_LEN],
                           uint8_t id,
                           uint8_t mode,
                           Machine_Direction dir,
                           uint8_t microstep,
                           uint16_t position_x10,
                           uint16_t speed_x10)
{
    if ((frame == 0) || !Machine_IsFrameParameterValid(mode, dir, microstep))
    {
        return 0U;
    }

    frame[0] = MACHINE_HEAD;
    frame[1] = id;
    frame[2] = mode;
    frame[3] = (uint8_t)dir;
    frame[4] = microstep;
    frame[5] = (uint8_t)(position_x10 >> 8);
    frame[6] = (uint8_t)(position_x10 & 0xFFU);
    frame[7] = (uint8_t)(speed_x10 >> 8);
    frame[8] = (uint8_t)(speed_x10 & 0xFFU);
    frame[9] = Machine_Checksum(frame, 9U);
    frame[10] = MACHINE_TAIL;

    return 1U;
}

/**
  * @brief  使用指定方向发送速度控制命令。
  * @param  id          电机设备地址。
  * @param  dir         转动方向。
  * @param  microstep   细分值。
  * @param  speed_rad_s 速度绝对值，单位 rad/s。
  * @retval HAL 库返回状态；参数错误时返回 HAL_ERROR。
  */
HAL_StatusTypeDef Machine_SetSpeedEx(uint8_t id,
                                     Machine_Direction dir,
                                     uint8_t microstep,
                                     float speed_rad_s)
{
    uint8_t frame[MACHINE_FRAME_LEN];
    uint16_t speed_x10 = Machine_Scale10Clamp(speed_rad_s);

    if (!Machine_BuildFrame(frame,
                            id,
                            MACHINE_MODE_SPEED,
                            dir,
                            microstep,
                            0U,
                            speed_x10))
    {
        return HAL_ERROR;
    }

    return Machine_SendFrame(frame);
}

/**
  * @brief  发送速度控制命令，速度符号用于指定方向。
  * @param  id          电机设备地址。
  * @param  speed_rad_s 有符号速度，正数为顺时针，负数为逆时针，单位 rad/s。
  * @param  microstep   细分值。
  * @retval HAL 库返回状态。
  */
HAL_StatusTypeDef Machine_SetSpeed(uint8_t id, float speed_rad_s, uint8_t microstep)
{
    Machine_Direction dir = (speed_rad_s >= 0.0f) ? MACHINE_DIR_CW : MACHINE_DIR_CCW;

    return Machine_SetSpeedEx(id, dir, microstep, speed_rad_s);
}

/**
  * @brief  发送力矩模式命令（控制模式 0x03）。
  * @param  id          电机设备地址。
  * @param  dir         转动方向。
  * @param  microstep   细分值。
  * @param  current_ma  电流绝对值，单位 mA。
  * @param  speed_rad_s 速度绝对值，单位 rad/s。
  * @retval HAL 库返回状态；参数错误时返回 HAL_ERROR。
  */
HAL_StatusTypeDef Machine_SetTorqueEx(uint8_t id,
                                      Machine_Direction dir,
                                      uint8_t microstep,
                                      float current_ma,
                                      float speed_rad_s)
{
    uint8_t frame[MACHINE_FRAME_LEN];
    uint16_t current_x1 = Machine_Scale10Clamp(current_ma / 10.0f);
    uint16_t speed_x10 = Machine_Scale10Clamp(speed_rad_s);

    if (!Machine_BuildFrame(frame,
                            id,
                            MACHINE_MODE_TORQUE,
                            dir,
                            microstep,
                            current_x1,
                            speed_x10))
    {
        return HAL_ERROR;
    }

    return Machine_SendFrame(frame);
}

/**
  * @brief  使用指定方向发送相对位置控制命令（控制模式 0x02）。
  * @param  id          电机设备地址。
  * @param  dir         转动方向。
  * @param  microstep   细分值，位置控制建议使用 32 细分。
  * @param  angle_deg   本次需要转过的角度绝对值，单位 deg。
  * @param  speed_rad_s 最大转速，单位 rad/s。
  * @retval HAL 库返回状态；参数错误时返回 HAL_ERROR。
  * @note   此接口表示“再转过指定角度”，不是转到单圈固定角度。
  */
HAL_StatusTypeDef Machine_SetPositionEx(uint8_t id,
                                        Machine_Direction dir,
                                        uint8_t microstep,
                                        float angle_deg,
                                        float speed_rad_s)
{
    uint8_t frame[MACHINE_FRAME_LEN];
    uint16_t position_x10 = Machine_Scale10Clamp(angle_deg);
    uint16_t speed_x10 = Machine_Scale10Clamp(speed_rad_s);

    if (!Machine_BuildFrame(frame,
                            id,
                            MACHINE_MODE_POSITION,
                            dir,
                            microstep,
                            position_x10,
                            speed_x10))
    {
        return HAL_ERROR;
    }

    return Machine_SendFrame(frame);
}

/**
  * @brief  发送相对位置控制命令，角度符号用于指定方向。
  * @param  id          电机设备地址。
  * @param  angle_deg   有符号相对角度，正数为顺时针，负数为逆时针，单位 deg。
  * @param  speed_rad_s 最大转速，单位 rad/s。
  * @param  microstep   细分值。
  * @retval HAL 库返回状态。
  */
HAL_StatusTypeDef Machine_SetPosition(uint8_t id,
                                      float angle_deg,
                                      float speed_rad_s,
                                      uint8_t microstep)
{
    Machine_Direction dir = (angle_deg >= 0.0f) ? MACHINE_DIR_CW : MACHINE_DIR_CCW;

    return Machine_SetPositionEx(id, dir, microstep, angle_deg, speed_rad_s);
}

/**
  * @brief  转到单圈绝对角度（控制模式 0x04）。
  * @param  id          电机设备地址。
  * @param  angle_deg   单圈目标角度，范围 0～360 deg。
  * @param  speed_rad_s 最大转速，单位 rad/s。
  * @retval HAL 库返回状态；角度越界时返回 HAL_ERROR。
  * @note   协议规定绝对角度模式固定使用 32 细分，方向由电机自动选择。
  */
HAL_StatusTypeDef Machine_SetAbsolutePosition(uint8_t id,
                                              float angle_deg,
                                              float speed_rad_s)
{
    uint8_t frame[MACHINE_FRAME_LEN];
    uint16_t position_x10;
    uint16_t speed_x10;

    if ((angle_deg < 0.0f) || (angle_deg > MACHINE_ABSOLUTE_ANGLE_MAX_DEG))
    {
        return HAL_ERROR;
    }

    position_x10 = Machine_Scale10Clamp(angle_deg);
    speed_x10 = Machine_Scale10Clamp(speed_rad_s);

    if (!Machine_BuildFrame(frame,
                            id,
                            MACHINE_MODE_ABSOLUTE_ANGLE,
                            MACHINE_DIR_CW,
                            MACHINE_DEFAULT_MICROSTEP,
                            position_x10,
                            speed_x10))
    {
        return HAL_ERROR;
    }

    return Machine_SendFrame(frame);
}

/**
  * @brief  转到单圈编码器绝对零位。
  * @param  id          电机设备地址。
  * @param  speed_rad_s 最大转速，单位 rad/s。
  * @retval HAL 库返回状态。
  * @note   此函数移动到编码器零位，不会重新标定或修改编码器零点。
  */
HAL_StatusTypeDef Machine_ReturnToZero(uint8_t id, float speed_rad_s)
{
    return Machine_SetAbsolutePosition(id, 0.0f, speed_rad_s);
}

/**
  * @brief  停止电机转动。
  * @param  id 电机设备地址。
  * @retval HAL 库返回状态。
  * @note   发送零速度命令后，电机是否保持力矩取决于驱动器参数。
  */
HAL_StatusTypeDef Machine_Stop(uint8_t id)
{
    return Machine_SetSpeedEx(id, MACHINE_DIR_CW, MACHINE_DEFAULT_MICROSTEP, 0.0f);
}

/**
  * @brief  兼容旧接口，使电机进入零速度保持状态。
  * @param  id 电机设备地址。
  * @retval HAL 库返回状态。
  * @note   MS42DC 控制协议没有独立的使能命令。
  */
HAL_StatusTypeDef Machine_Enable(uint8_t id)
{
    return Machine_Stop(id);
}

/**
  * @brief  兼容旧接口，停止电机转动。
  * @param  id 电机设备地址。
  * @retval HAL 库返回状态。
  * @note   MS42DC 控制协议没有独立的失能命令，本函数不保证关闭保持力矩。
  */
HAL_StatusTypeDef Machine_Disable(uint8_t id)
{
    return Machine_Stop(id);
}

/**
  * @brief  请求电机返回实时速度和位置数据。
  * @param  id 电机设备地址。
  * @retval HAL 库返回状态。
  * @note   请求帧固定为 7B ID 00 00 00 00 00 00 00 BCC 7D。
  */
HAL_StatusTypeDef Machine_RequestFeedback(uint8_t id)
{
    uint8_t frame[MACHINE_FRAME_LEN];

    if (!Machine_BuildFrame(frame,
                            id,
                            MACHINE_MODE_FEEDBACK,
                            MACHINE_DIR_CCW,
                            0U,
                            0U,
                            0U))
    {
        return HAL_ERROR;
    }

    return Machine_SendFrame(frame);
}

/**
  * @brief  解析电机返回的 9 字节 TTL 反馈帧。
  * @param  frame       TTL 反馈帧，格式见手册表 3-16。
  * @param  expected_id 期望的电机设备地址。
  * @param  feedback    解析结果输出地址。
  * @retval 1 表示地址和 BCC 校验均正确；0 表示帧无效。
  * @note   速度为有符号 16 位数据，位置为有符号 32 位数据，均放大 10 倍。
  */
uint8_t Machine_ParseFeedbackFrame(const uint8_t frame[MACHINE_FEEDBACK_FRAME_LEN],
                                   uint8_t expected_id,
                                   Machine_Feedback *feedback)
{
    uint16_t speed_raw;
    uint32_t position_raw;

    if ((frame == 0) || (feedback == 0) || (frame[0] != expected_id))
    {
        return 0U;
    }

    if (Machine_Checksum(frame, MACHINE_FEEDBACK_FRAME_LEN - 1U) !=
        frame[MACHINE_FEEDBACK_FRAME_LEN - 1U])
    {
        return 0U;
    }

    speed_raw = ((uint16_t)frame[2] << 8) | (uint16_t)frame[3];
    position_raw = ((uint32_t)frame[4] << 24) |
                   ((uint32_t)frame[5] << 16) |
                   ((uint32_t)frame[6] << 8) |
                   (uint32_t)frame[7];

    feedback->id = frame[0];
    feedback->target_reached = frame[1];
    memcpy(&feedback->speed_x10, &speed_raw, sizeof(feedback->speed_x10));
    memcpy(&feedback->position_x10, &position_raw, sizeof(feedback->position_x10));

    return 1U;
}

/**
  * @brief 为 USART2 挂接下一字节中断接收。
  */
static HAL_StatusTypeDef Machine_FeedbackRxArm(void)
{
    return HAL_UART_Receive_IT(&huart2, &machine_feedback_rx_byte, 1U);
}

/**
  * @brief 启动 USART2反馈的持续中断接收。
  */
HAL_StatusTypeDef Machine_FeedbackRxStart(void)
{
    (void)HAL_UART_AbortReceive(&huart2);
    __HAL_UART_CLEAR_PEFLAG(&huart2);
    huart2.ErrorCode = HAL_UART_ERROR_NONE;

    machine_feedback_rx_count = 0U;
    machine_feedback_sequence = 0U;
    machine_feedback_available = 0U;
    machine_uart2_rx_byte_count = 0U;
    machine_feedback_valid_frame_count = 0U;
    machine_feedback_reject_count = 0U;
    machine_uart2_error_count = 0U;

    return Machine_FeedbackRxArm();
}

/**
  * @brief 处理 USART2 收到的一个反馈字节并立即重新挂接接收。
  * @note 反馈没有帧头和帧尾，因此使用9字节滑窗查找地址和BCC均合法的帧。
  */
void Machine_FeedbackRxByteCallback(void)
{
    Machine_Feedback parsed_feedback;
    uint8_t i;

    machine_uart2_rx_byte_count++;

    if (machine_feedback_rx_count < MACHINE_FEEDBACK_FRAME_LEN)
    {
        machine_feedback_rx_window[machine_feedback_rx_count++] =
            machine_feedback_rx_byte;
    }

    if (machine_feedback_rx_count == MACHINE_FEEDBACK_FRAME_LEN)
    {
        if (Machine_ParseFeedbackFrame(machine_feedback_rx_window,
                                       MACHINE_DEFAULT_ID,
                                       &parsed_feedback))
        {
            machine_feedback_latest.id = parsed_feedback.id;
            machine_feedback_latest.target_reached = parsed_feedback.target_reached;
            machine_feedback_latest.speed_x10 = parsed_feedback.speed_x10;
            machine_feedback_latest.position_x10 = parsed_feedback.position_x10;
            machine_feedback_available = 1U;
            machine_feedback_valid_frame_count++;

            machine_feedback_sequence++;
            if (machine_feedback_sequence == 0U)
            {
                machine_feedback_sequence = 1U;
            }

            /* 已找到完整帧，下一字节就是下一帧的地址。 */
            machine_feedback_rx_count = 0U;
        }
        else
        {
            machine_feedback_reject_count++;
            /* 当前窗口无效，左移一字节继续自动寻找帧边界。 */
            for (i = 1U; i < MACHINE_FEEDBACK_FRAME_LEN; i++)
            {
                machine_feedback_rx_window[i - 1U] =
                    machine_feedback_rx_window[i];
            }
            machine_feedback_rx_count = MACHINE_FEEDBACK_FRAME_LEN - 1U;
        }
    }

    (void)Machine_FeedbackRxArm();
}

/**
  * @brief USART2接收错误后清除半帧并恢复持续接收。
  */
void Machine_FeedbackRxErrorCallback(void)
{
    machine_uart2_error_count++;
    (void)HAL_UART_AbortReceive(&huart2);
    __HAL_UART_CLEAR_PEFLAG(&huart2);
    huart2.ErrorCode = HAL_UART_ERROR_NONE;
    machine_feedback_rx_count = 0U;
    (void)Machine_FeedbackRxArm();
}

/**
  * @brief 读取中断缓存中的最近一帧合法反馈。
  */
uint8_t Machine_GetLatestFeedback(Machine_Feedback *feedback,
                                  uint32_t *sequence)
{
    uint32_t primask;

    if (feedback == 0)
    {
        return 0U;
    }

    primask = __get_PRIMASK();
    __disable_irq();

    if (!machine_feedback_available)
    {
        if (sequence != 0)
        {
            *sequence = 0U;
        }
        if (primask == 0U)
        {
            __enable_irq();
        }
        return 0U;
    }

    feedback->id = machine_feedback_latest.id;
    feedback->target_reached = machine_feedback_latest.target_reached;
    feedback->speed_x10 = machine_feedback_latest.speed_x10;
    feedback->position_x10 = machine_feedback_latest.position_x10;
    if (sequence != 0)
    {
        *sequence = machine_feedback_sequence;
    }

    if (primask == 0U)
    {
        __enable_irq();
    }

    return 1U;
}

/**
  * @brief 在持续中断接收保持开启的状态下，请求并等待下一帧合法反馈。
  * @param id         期望的设备地址。
  * @param feedback   新反馈输出地址。
  * @param timeout_ms 等待新反馈的最长时间，单位ms。
  * @retval HAL_OK收到新反馈；HAL_TIMEOUT超时；HAL_ERROR参数错误。
  */
HAL_StatusTypeDef Machine_ReadFeedback(uint8_t id,
                                       Machine_Feedback *feedback,
                                       uint32_t timeout_ms)
{
    Machine_Feedback latest_feedback;
    uint32_t start_sequence = 0U;
    uint32_t current_sequence = 0U;
    uint32_t start_tick;
    HAL_StatusTypeDef status;

    if ((feedback == 0) || (timeout_ms == 0U))
    {
        return HAL_ERROR;
    }

    (void)Machine_GetLatestFeedback(&latest_feedback, &start_sequence);

    /* 接收中断已经提前挂好，此处只发送请求，不会错过立即返回的首字节。 */
    status = Machine_RequestFeedback(id);
    if (status != HAL_OK)
    {
        return status;
    }

    start_tick = HAL_GetTick();

    while ((HAL_GetTick() - start_tick) < timeout_ms)
    {
        if (Machine_GetLatestFeedback(&latest_feedback, &current_sequence) &&
            (current_sequence != start_sequence) &&
            (latest_feedback.id == id))
        {
            *feedback = latest_feedback;
            return HAL_OK;
        }

        HAL_Delay(1U);
    }

    return HAL_TIMEOUT;
}
