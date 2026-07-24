#ifndef MACHINE_H
#define MACHINE_H

#include <stdint.h>

/*******************************************************************************
 * 使用示例：
 *
 * 1. 调用本驱动前，必须先执行系统初始化。SYSCFG_DL_init() 会按照
 *    SysConfig 配置完成 UART_Machine 的 UART4、PB10/PB11 和 115200 8N1
 *    初始化，因此不需要再单独初始化 UART_Machine。
 *
 *        SYSCFG_DL_init();
 *
 * 2. 将 X 轴初始化为多圈 T 型位置模式。驱动会依次设置控制模式、
 *    设置 20 RPM 速度、设置 21.0 度初始目标位置并使能电机。
 *
 *        Machine_InitAxis(MACHINE_AXIS_X, MACHINE_MODE_MULTI_TURN_T);
 *
 * 3. 将 Y 轴初始化为单圈 T 型位置模式。Y 轴不支持多圈位置模式。
 *
 *        Machine_InitAxis(MACHINE_AXIS_Y, MACHINE_MODE_SINGLE_TURN_T);
 *
 * 4. 控制 X 轴移动到多圈位置 360.0 度。位置参数单位为 0.1 度，
 *    因此 360.0 度需要传入 3600。
 *
 *        Machine_SetMultiTurnPosition(3600);
 *
 * 5. 控制 Y 轴移动到单圈位置 21.0 度。位置参数单位为 0.1 度，
 *    因此 21.0 度需要传入 210。
 *
 *        Machine_SetSingleTurnPosition(MACHINE_AXIS_Y, 210U);
 *
 * 6. 需要同时修改位置模式下的运动速度和目标位置时，应使用组合接口。
 *    下例先将 Y 轴速度设置为 20 RPM，等待 10 ms，再发送 90.0 度目标位置。
 *
 *        Machine_SetSingleTurnMotion(MACHINE_AXIS_Y, 20, 900U);
 *
 *    下例先将 X 轴速度设置为 20 RPM，等待 10 ms，再发送 -360.0 度
 *    多圈目标位置。
 *
 *        Machine_SetMultiTurnMotion(20, -3600);
 *
 * 7. 速度模式初始化和正反转控制示例。正速度表示正转，负速度表示
 *    反转，速度 0 表示停止。
 *
 *        Machine_InitAxis(MACHINE_AXIS_Y, MACHINE_MODE_SPEED);
 *        Machine_SetSpeed(MACHINE_AXIS_Y, 20);
 *        Machine_SetSpeed(MACHINE_AXIS_Y, -20);
 *        Machine_Stop(MACHINE_AXIS_Y);
 *
 * 8. 不再需要电机输出时，可以发送失能命令；重新使用前发送使能命令。
 *
 *        Machine_Disable(MACHINE_AXIS_Y);
 *        Machine_Enable(MACHINE_AXIS_Y);
 *
 * 注意事项：
 * 1. 初始化函数内部已经按照协议加入 10 ms 指令间隔和使能后的等待时间。
 * 2. 两个组合控制接口内部已经在速度与位置命令之间等待 10 ms。
 * 3. 单独调用控制接口时，由上层任务保证连续控制帧间隔不小于 5 ms，
 *    推荐以 10 ms 周期调用，即控制频率为 100 Hz。
 * 4. 本驱动使用阻塞式串口发送，不可在多个任务或中断中同时调用。
 *******************************************************************************/

/* 二维云台电机的默认总线地址。 */
#define MACHINE_X_ADDRESS                    (0x01U)
#define MACHINE_Y_ADDRESS                    (0x02U)

/* 驱动初始化时使用的安全默认参数。 */
#define MACHINE_DEFAULT_SPEED_RPM            (20)
#define MACHINE_DEFAULT_X_POSITION_0P1_DEG   (210)

/* 单圈位置的协议范围，单位为 0.1 度。 */
#define MACHINE_SINGLE_POSITION_MIN_0P1_DEG  (0U)
#define MACHINE_SINGLE_POSITION_MAX_0P1_DEG  (3599U)

/* 协议规定的指令间隔，单位为毫秒。 */
#define MACHINE_COMMAND_INTERVAL_MS          (10U)
#define MACHINE_ENABLE_WAIT_MS               (15U)

/**
 * @brief 二维云台轴定义，枚举值与电机地址一致。
 */
typedef enum {
    MACHINE_AXIS_X = MACHINE_X_ADDRESS,
    MACHINE_AXIS_Y = MACHINE_Y_ADDRESS
} Machine_Axis_t;

/**
 * @brief 电机控制模式定义。
 */
typedef enum {
    MACHINE_MODE_SPEED                 = 0x0000U,
    MACHINE_MODE_MULTI_TURN_T          = 0x0001U,
    MACHINE_MODE_SINGLE_TURN_T         = 0x0002U,
    MACHINE_MODE_MULTI_TURN_DIRECT     = 0x0003U,
    MACHINE_MODE_SINGLE_TURN_DIRECT    = 0x0004U
} Machine_Mode_t;

/**
 * @brief Machine 驱动接口返回状态。
 */
typedef enum {
    MACHINE_STATUS_OK = 0,
    MACHINE_STATUS_INVALID_AXIS,
    MACHINE_STATUS_INVALID_MODE,
    MACHINE_STATUS_INVALID_POSITION
} Machine_Status_t;

/*******************************************************************************
 * 名    称： Machine_InitAxis
 * 功    能：按照指定模式完成单个云台轴的安全初始化。
 * 参    数：axis：X 轴或 Y 轴；mode：目标控制模式。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 * 说    明：调用前必须先执行 SYSCFG_DL_init()。位置模式默认速度为 20 RPM；
 *           X 轴多圈模式在使能前将默认目标设置为 21.0 度；速度模式以
 *           0 RPM 使能。Y 轴不支持多圈模式。
 *******************************************************************************/
Machine_Status_t Machine_InitAxis(Machine_Axis_t axis, Machine_Mode_t mode);

/*******************************************************************************
 * 名    称： Machine_SetMode
 * 功    能：设置指定云台轴的控制模式。
 * 参    数：axis：X 轴或 Y 轴；mode：目标控制模式。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 * 说    明：Y 轴不允许设置多圈 T 型或多圈直通模式。
 *******************************************************************************/
Machine_Status_t Machine_SetMode(Machine_Axis_t axis, Machine_Mode_t mode);

/*******************************************************************************
 * 名    称： Machine_SetSpeed
 * 功    能：设置指定云台轴的转速，速度模式下可直接用于正反转控制。
 * 参    数：axis：X 轴或 Y 轴；speedRpm：有符号 16 位转速，单位为 RPM。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 * 说    明：正数表示正转，负数表示反转，0 表示停止。
 *******************************************************************************/
Machine_Status_t Machine_SetSpeed(Machine_Axis_t axis, int16_t speedRpm);

/*******************************************************************************
 * 名    称： Machine_SetSingleTurnPosition
 * 功    能：设置指定云台轴的单圈目标位置。
 * 参    数：axis：X 轴或 Y 轴；position0p1Deg：目标角度，单位为 0.1 度。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 * 说    明：有效范围为 0～3599，对应 0.0～359.9 度。例如 21.0 度传入 210。
 *******************************************************************************/
Machine_Status_t Machine_SetSingleTurnPosition(Machine_Axis_t axis,
                                                uint16_t position0p1Deg);

/*******************************************************************************
 * 名    称： Machine_SetMultiTurnPosition
 * 功    能：设置 X 轴的多圈目标位置。
 * 参    数：position0p1Deg：有符号目标角度，单位为 0.1 度。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 * 说    明：该协议仅适用于 X 轴。例如 -360.0 度传入 -3600。
 *******************************************************************************/
Machine_Status_t Machine_SetMultiTurnPosition(int32_t position0p1Deg);

/*******************************************************************************
 * 名    称： Machine_SetSingleTurnMotion
 * 功    能：先更新速度，等待协议间隔，再设置单圈目标位置。
 * 参    数：axis：X 轴或 Y 轴；speedRpm：转速，单位为 RPM；
 *           position0p1Deg：目标角度，单位为 0.1 度。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetSingleTurnMotion(Machine_Axis_t axis,
                                              int16_t speedRpm,
                                              uint16_t position0p1Deg);

/*******************************************************************************
 * 名    称： Machine_SetMultiTurnMotion
 * 功    能：先更新速度，等待协议间隔，再设置 X 轴多圈目标位置。
 * 参    数：speedRpm：转速，单位为 RPM；position0p1Deg：目标角度，
 *           单位为 0.1 度。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_SetMultiTurnMotion(int16_t speedRpm,
                                             int32_t position0p1Deg);

/*******************************************************************************
 * 名    称： Machine_Enable
 * 功    能：使能指定云台轴电机。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Enable(Machine_Axis_t axis);

/*******************************************************************************
 * 名    称： Machine_Disable
 * 功    能：失能指定云台轴电机。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Disable(Machine_Axis_t axis);

/*******************************************************************************
 * 名    称： Machine_Stop
 * 功    能：向指定云台轴发送 0 RPM 速度指令。
 * 参    数：axis：X 轴或 Y 轴。
 * 出    口：Machine_Status_t，成功返回 MACHINE_STATUS_OK。
 *******************************************************************************/
Machine_Status_t Machine_Stop(Machine_Axis_t axis);

#endif
