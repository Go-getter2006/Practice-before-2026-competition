#include "Machine.h"
#include "Delay.h"
#include "UART.h"

#define MACHINE_FRAME_HEADER       (0x7AU)
#define MACHINE_FRAME_TAIL         (0x7BU)
#define MACHINE_FRAME_MAX_LENGTH   (9U)

#define MACHINE_CMD_SET_MODE       (0x00U)
#define MACHINE_CMD_SET_SPEED      (0x01U)
#define MACHINE_CMD_MULTI_POSITION (0x02U)
#define MACHINE_CMD_SINGLE_POSITION (0x03U)
#define MACHINE_CMD_DISABLE        (0x05U)
#define MACHINE_CMD_ENABLE         (0x06U)

/*******************************************************************************
 * 名    称： Machine_IsValidAxis
 * 功    能：检查轴枚举是否对应已配置的 X 轴或 Y 轴地址。
 * 参    数：axis：待检查的轴枚举。
 * 出    口：有效返回 1U，否则返回 0U。
 *******************************************************************************/
static uint8_t Machine_IsValidAxis(Machine_Axis_t axis)
{
    return (uint8_t)((axis == MACHINE_AXIS_X) || (axis == MACHINE_AXIS_Y));
}

/*******************************************************************************
 * 名    称： Machine_IsMultiTurnMode
 * 功    能：判断指定模式是否为多圈位置模式。
 * 参    数：mode：待检查的控制模式。
 * 出    口：多圈模式返回 1U，否则返回 0U。
 *******************************************************************************/
static uint8_t Machine_IsMultiTurnMode(Machine_Mode_t mode)
{
    return (uint8_t)((mode == MACHINE_MODE_MULTI_TURN_T) ||
                     (mode == MACHINE_MODE_MULTI_TURN_DIRECT));
}

/*******************************************************************************
 * 名    称： Machine_IsSingleTurnMode
 * 功    能：判断指定模式是否为单圈位置模式。
 * 参    数：mode：待检查的控制模式。
 * 出    口：单圈模式返回 1U，否则返回 0U。
 *******************************************************************************/
static uint8_t Machine_IsSingleTurnMode(Machine_Mode_t mode)
{
    return (uint8_t)((mode == MACHINE_MODE_SINGLE_TURN_T) ||
                     (mode == MACHINE_MODE_SINGLE_TURN_DIRECT));
}

/*******************************************************************************
 * 名    称： Machine_IsValidMode
 * 功    能：检查控制模式是否有效，并限制 Y 轴不能使用多圈模式。
 * 参    数：axis：目标轴；mode：待检查的控制模式。
 * 出    口：有效返回 1U，否则返回 0U。
 *******************************************************************************/
static uint8_t Machine_IsValidMode(Machine_Axis_t axis, Machine_Mode_t mode)
{
    uint8_t isKnownMode;

    isKnownMode = (uint8_t)((mode == MACHINE_MODE_SPEED) ||
                            Machine_IsSingleTurnMode(mode) ||
                            Machine_IsMultiTurnMode(mode));

    if ((isKnownMode == 0U) ||
        ((axis == MACHINE_AXIS_Y) && (Machine_IsMultiTurnMode(mode) != 0U))) {
        return 0U;
    }

    return 1U;
}

/*******************************************************************************
 * 名    称： Machine_SendFrame
 * 功    能：组装云台协议帧、计算 BCC 校验并通过 UART_Machine 发送。
 * 参    数：axis：目标轴；command：功能码；data：数据区；
 *           dataLength：数据区字节数，只允许 0、2 或 4。
 * 出    口：无返回值。
 * 说    明：BCC 为帧头、地址、功能码和全部数据字节的异或结果。
 *******************************************************************************/
static void Machine_SendFrame(Machine_Axis_t axis,
                              uint8_t command,
                              const uint8_t *data,
                              uint8_t dataLength)
{
    uint8_t frame[MACHINE_FRAME_MAX_LENGTH];
    uint8_t bcc;
    uint8_t index;
    uint8_t frameLength;

    frame[0] = MACHINE_FRAME_HEADER;
    frame[1] = (uint8_t)axis;
    frame[2] = command;

    for (index = 0U; index < dataLength; index++) {
        frame[3U + index] = data[index];
    }

    bcc = frame[0];
    for (index = 1U; index < (uint8_t)(3U + dataLength); index++) {
        bcc ^= frame[index];
    }

    frame[3U + dataLength] = bcc;
    frame[4U + dataLength] = MACHINE_FRAME_TAIL;
    frameLength = (uint8_t)(5U + dataLength);
    UART_Machine_SendData(frame, frameLength);
}

/*******************************************************************************
 * 名    称： Machine_InitAxis
 * 功    能：按照协议规定的顺序和间隔完成指定云台轴初始化。
 * 参    数：axis：X 轴或 Y 轴；mode：目标控制模式。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_InitAxis(Machine_Axis_t axis, Machine_Mode_t mode)
{
    Machine_Status_t status;

    status = Machine_SetMode(axis, mode);
    if (status != MACHINE_STATUS_OK) {
        return status;
    }
    Delay_ms(MACHINE_COMMAND_INTERVAL_MS);

    if (mode == MACHINE_MODE_SPEED) {
        status = Machine_SetSpeed(axis, 0);
    } else {
        status = Machine_SetSpeed(axis, MACHINE_DEFAULT_SPEED_RPM);
    }
    if (status != MACHINE_STATUS_OK) {
        return status;
    }
    Delay_ms(MACHINE_COMMAND_INTERVAL_MS);

    if (Machine_IsMultiTurnMode(mode) != 0U) {
        status = Machine_SetMultiTurnPosition(MACHINE_DEFAULT_X_POSITION_0P1_DEG);
        if (status != MACHINE_STATUS_OK) {
            return status;
        }
        Delay_ms(MACHINE_COMMAND_INTERVAL_MS);
    }

    status = Machine_Enable(axis);
    if (status != MACHINE_STATUS_OK) {
        return status;
    }
    Delay_ms(MACHINE_ENABLE_WAIT_MS);

    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_SetMode
 * 功    能：发送控制模式设置帧。
 * 参    数：axis：X 轴或 Y 轴；mode：目标控制模式。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetMode(Machine_Axis_t axis, Machine_Mode_t mode)
{
    uint16_t modeValue;
    uint8_t data[2];

    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }
    if (Machine_IsValidMode(axis, mode) == 0U) {
        return MACHINE_STATUS_INVALID_MODE;
    }

    modeValue = (uint16_t)mode;
    data[0] = (uint8_t)(modeValue >> 8U);
    data[1] = (uint8_t)modeValue;
    Machine_SendFrame(axis, MACHINE_CMD_SET_MODE, data, 2U);

    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_SetSpeed
 * 功    能：将有符号 RPM 按 16 位补码、高字节在前的格式发送。
 * 参    数：axis：X 轴或 Y 轴；speedRpm：目标转速，单位为 RPM。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetSpeed(Machine_Axis_t axis, int16_t speedRpm)
{
    uint16_t speedValue;
    uint8_t data[2];

    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }

    speedValue = (uint16_t)speedRpm;
    data[0] = (uint8_t)(speedValue >> 8U);
    data[1] = (uint8_t)speedValue;
    Machine_SendFrame(axis, MACHINE_CMD_SET_SPEED, data, 2U);

    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_SetSingleTurnPosition
 * 功    能：发送单圈目标位置，协议单位为 0.1 度。
 * 参    数：axis：X 轴或 Y 轴；position0p1Deg：目标位置。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetSingleTurnPosition(Machine_Axis_t axis,
                                                uint16_t position0p1Deg)
{
    uint8_t data[2];

    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }
    if (position0p1Deg > MACHINE_SINGLE_POSITION_MAX_0P1_DEG) {
        return MACHINE_STATUS_INVALID_POSITION;
    }

    data[0] = (uint8_t)(position0p1Deg >> 8U);
    data[1] = (uint8_t)position0p1Deg;
    Machine_SendFrame(axis, MACHINE_CMD_SINGLE_POSITION, data, 2U);

    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_SetMultiTurnPosition
 * 功    能：发送 X 轴多圈目标位置，使用有符号 32 位补码格式。
 * 参    数：position0p1Deg：目标位置，单位为 0.1 度。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetMultiTurnPosition(int32_t position0p1Deg)
{
    uint32_t positionValue;
    uint8_t data[4];

    positionValue = (uint32_t)position0p1Deg;
    data[0] = (uint8_t)(positionValue >> 24U);
    data[1] = (uint8_t)(positionValue >> 16U);
    data[2] = (uint8_t)(positionValue >> 8U);
    data[3] = (uint8_t)positionValue;
    Machine_SendFrame(MACHINE_AXIS_X, MACHINE_CMD_MULTI_POSITION, data, 4U);

    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_SetSingleTurnMotion
 * 功    能：按“速度、等待 10 ms、单圈位置”的顺序发送组合控制命令。
 * 参    数：axis：X 轴或 Y 轴；speedRpm：目标转速；
 *           position0p1Deg：单圈目标位置。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetSingleTurnMotion(Machine_Axis_t axis,
                                              int16_t speedRpm,
                                              uint16_t position0p1Deg)
{
    Machine_Status_t status;

    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }
    if (position0p1Deg > MACHINE_SINGLE_POSITION_MAX_0P1_DEG) {
        return MACHINE_STATUS_INVALID_POSITION;
    }

    status = Machine_SetSpeed(axis, speedRpm);
    if (status != MACHINE_STATUS_OK) {
        return status;
    }
    Delay_ms(MACHINE_COMMAND_INTERVAL_MS);

    return Machine_SetSingleTurnPosition(axis, position0p1Deg);
}

/*******************************************************************************
 * 名    称： Machine_SetMultiTurnMotion
 * 功    能：按“速度、等待 10 ms、多圈位置”的顺序发送 X 轴组合命令。
 * 参    数：speedRpm：目标转速；position0p1Deg：多圈目标位置。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetMultiTurnMotion(int16_t speedRpm,
                                             int32_t position0p1Deg)
{
    Machine_Status_t status;

    status = Machine_SetSpeed(MACHINE_AXIS_X, speedRpm);
    if (status != MACHINE_STATUS_OK) {
        return status;
    }
    Delay_ms(MACHINE_COMMAND_INTERVAL_MS);

    return Machine_SetMultiTurnPosition(position0p1Deg);
}

/*******************************************************************************
 * 名    称： Machine_Enable
 * 功    能：发送无数据区的电机使能帧。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Enable(Machine_Axis_t axis)
{
    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }

    Machine_SendFrame(axis, MACHINE_CMD_ENABLE, (const uint8_t *)0, 0U);
    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_Disable
 * 功    能：发送无数据区的电机失能帧。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Disable(Machine_Axis_t axis)
{
    if (Machine_IsValidAxis(axis) == 0U) {
        return MACHINE_STATUS_INVALID_AXIS;
    }

    Machine_SendFrame(axis, MACHINE_CMD_DISABLE, (const uint8_t *)0, 0U);
    return MACHINE_STATUS_OK;
}

/*******************************************************************************
 * 名    称： Machine_Stop
 * 功    能：向指定云台轴发送 0 RPM 指令。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Stop(Machine_Axis_t axis)
{
    return Machine_SetSpeed(axis, 0);
}
