#include "Motor.h"
#include "ti_msp_dl_config.h"

/* 最近一个采样周期内的左右轮 QEI 增量，以及自初始化以来的累计计数。 */
volatile int16_t Motor_LeftSpeed = 0;
volatile int16_t Motor_RightSpeed = 0;
volatile int32_t Motor_LeftEncoderTotal = 0;
volatile int32_t Motor_RightEncoderTotal = 0;

/* GPIO 外部中断累计的本采样周期计数和上电后总计数。 */
volatile int32_t Encoder_Left_Count = 0;
volatile int32_t Encoder_Right_Count = 0;
volatile int32_t Get_Encoder_countA = 0;
volatile int32_t Get_Encoder_countB = 0;

#if 0
/* 旧硬件 QEI 调试变量，切换回硬件 QEI 时可重新启用。 */
volatile uint32_t Motor_LeftDirectionChanges = 0U;
volatile uint32_t Motor_RightDirectionChanges = 0U;
#endif

/* Motor_Init() 完成后置位，防止定时中断在初始化完成前读取编码器计数。 */
static volatile uint8_t motor_encoder_ready = 0U;

#if 0
/* 上一个采样周期读取到的 QEI 自由运行计数值。 */
static uint16_t motor_left_encoder_previous = 0U;
static uint16_t motor_right_encoder_previous = 0U;

/*******************************************************************************
 * 函数名称：Motor_ReadEncoderDelta
 * 功    能：根据 QEI 当前计数与上一次计数之差，计算本采样周期的有符号增量。
 * 实现说明：QEI 保持自由运行，不在运行期间直接修改计数寄存器。两个 uint16_t
 *           计数值相减后转换为 int16_t，可自动处理 16 位计数器的上下回绕；
 *           reverse 非零时再反转结果符号。
 * 参    数：encoder：QEI 定时器寄存器地址；previous：上次计数值保存地址；
 *           reverse：是否反转计数符号。
 * 返 回 值：本采样周期内的有符号 QEI 增量计数。
 *******************************************************************************/
static int16_t Motor_ReadEncoderDelta(
    GPTIMER_Regs *encoder, uint16_t *previous, uint8_t reverse)
{
    uint16_t current;
    int16_t delta;

    current = (uint16_t)DL_TimerG_getTimerCount(encoder);
    delta = (int16_t)(uint16_t)(current - *previous);
    *previous = current;

    if (reverse != 0U) {
        delta = (int16_t)(-(int32_t)delta);
    }

    return delta;
}
#endif

/*******************************************************************************
 * 函数名称：Motor_Clamp
 * 功    能：将电机驱动指令限制在允许的 PWM 范围内。
 * 参    数：speed：待限幅的有符号驱动指令。
 * 返 回 值：限幅后的驱动指令。
 ******************************************************************************/
static int16_t Motor_Clamp(int16_t speed)
{
    if (speed > (int16_t)MOTOR_PWM_MAX) {
        speed = (int16_t)MOTOR_PWM_MAX;
    } else if (speed < -(int16_t)MOTOR_PWM_MAX) {
        speed = -(int16_t)MOTOR_PWM_MAX;
    }

    return speed;
}

/*******************************************************************************
 * 函数名称：Motor_Init
 * 功    能：初始化 TB6612 的两组方向输入、停止两路电机并清零编码器数据。
 * 说    明：GPIO 输出方向和 PWM 外设已由 SysConfig 配置。AIN1/AIN2 对应左轮，
 *           BIN1/BIN2 对应右轮；初始化时将四个方向输入全部置为低电平。
 * 参    数：无。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_Init(void)
{
    DL_GPIO_clearPins(AIN_PORT, AIN_AIN1_PIN | AIN_AIN2_PIN);
    DL_GPIO_clearPins(BIN_PORT, BIN_BIN1_PIN | BIN_BIN2_PIN);
    Motor_Stop();
    Motor_ResetEncoderTotal();

#if 0
    /* 按照 TI QEI 例程显式确认计数器已启动，并使能方向变化中断。 */
    if (!DL_TimerG_isRunning(QEI_LEFT_INST)) {
        DL_TimerG_startCounter(QEI_LEFT_INST);
    }
    if (!DL_TimerG_isRunning(QEI_RIGHT_INST)) {
        DL_TimerG_startCounter(QEI_RIGHT_INST);
    }
    NVIC_ClearPendingIRQ(QEI_LEFT_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(QEI_RIGHT_INST_INT_IRQN);
    NVIC_EnableIRQ(QEI_LEFT_INST_INT_IRQN);
    NVIC_EnableIRQ(QEI_RIGHT_INST_INT_IRQN);
#endif

    motor_encoder_ready = 1U;
}

/*******************************************************************************
 * 函数名称：Motor_SetLeft
 * 功    能：设置 TB6612 A 通道所连接的左轮电机。
 * 控制逻辑：正向时 AIN1=0、AIN2=1；反向时 AIN1=1、AIN2=0；停止时
 *           AIN1=0、AIN2=0，电机进入短刹车状态。
 * PWM 说明：PWMA 使用 TIMA0_CC0。定时器采用向下计数的边沿对齐模式，
 *           因此比较值等于 MOTOR_PWM_MAX 减去所需的有效占空比计数。
 * 参    数：speed：左轮有符号驱动指令，绝对值范围为 0～MOTOR_PWM_MAX。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_SetLeft(int16_t speed)
{
    uint16_t duty;
    uint16_t compare;

    speed = Motor_Clamp(speed);

    if (speed > 0) {
        DL_GPIO_clearPins(AIN_PORT, AIN_AIN1_PIN);
        DL_GPIO_setPins(AIN_PORT, AIN_AIN2_PIN);
        duty = (uint16_t)speed;
    } else if (speed < 0) {
        DL_GPIO_setPins(AIN_PORT, AIN_AIN1_PIN);
        DL_GPIO_clearPins(AIN_PORT, AIN_AIN2_PIN);
        duty = (uint16_t)(-speed);
    } else {
        DL_GPIO_clearPins(AIN_PORT, AIN_AIN1_PIN | AIN_AIN2_PIN);
        duty = 0U;
    }

    compare = (uint16_t)MOTOR_PWM_MAX - duty;
    DL_TimerA_setCaptureCompareValue(
        PWM_Motor_INST, compare, DL_TIMER_CC_0_INDEX);
}

/*******************************************************************************
 * 函数名称：Motor_SetRight
 * 功    能：设置 TB6612 B 通道所连接的右轮电机。
 * 控制逻辑：正向时 BIN1=0、BIN2=1；反向时 BIN1=1、BIN2=0；停止时
 *           BIN1=0、BIN2=0，电机进入短刹车状态。
 * PWM 说明：PWMB 使用 TIMA0_CC1。定时器采用向下计数的边沿对齐模式，
 *           因此比较值等于 MOTOR_PWM_MAX 减去所需的有效占空比计数。
 * 参    数：speed：右轮有符号驱动指令，绝对值范围为 0～MOTOR_PWM_MAX。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_SetRight(int16_t speed)
{
    uint16_t duty;
    uint16_t compare;

    speed = Motor_Clamp(speed);

    if (speed > 0) {
        DL_GPIO_clearPins(BIN_PORT, BIN_BIN1_PIN);
        DL_GPIO_setPins(BIN_PORT, BIN_BIN2_PIN);
        duty = (uint16_t)speed;
    } else if (speed < 0) {
        DL_GPIO_setPins(BIN_PORT, BIN_BIN1_PIN);
        DL_GPIO_clearPins(BIN_PORT, BIN_BIN2_PIN);
        duty = (uint16_t)(-speed);
    } else {
        DL_GPIO_clearPins(BIN_PORT, BIN_BIN1_PIN | BIN_BIN2_PIN);
        duty = 0U;
    }

    compare = (uint16_t)MOTOR_PWM_MAX - duty;
    DL_TimerA_setCaptureCompareValue(
        PWM_Motor_INST, compare, DL_TIMER_CC_1_INDEX);
}

/*******************************************************************************
 * 函数名称：Motor_SetSpeed
 * 功    能：同时设置左右轮电机的有符号驱动指令。
 * 参    数：left：左轮驱动指令；right：右轮驱动指令。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_SetSpeed(int16_t left, int16_t right)
{
    Motor_SetLeft(left);
    Motor_SetRight(right);
}

/*******************************************************************************
 * 函数名称：Motor_Forward
 * 功    能：控制小车以指定 PWM 幅值前进。
 * 说    明：左右电机镜像安装，因此前进时左轮使用正向指令，右轮使用反向指令。
 * 参    数：speed：PWM 幅值，范围为 0～MOTOR_PWM_MAX。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_Forward(uint16_t speed)
{
    int16_t clamped;

    clamped = Motor_Clamp((int16_t)speed);
    Motor_SetSpeed(clamped, -clamped);
}

/*******************************************************************************
 * 函数名称：Motor_Backward
 * 功    能：控制小车以指定 PWM 幅值后退。
 * 说    明：左右电机镜像安装，因此后退时左轮使用反向指令，右轮使用正向指令。
 * 参    数：speed：PWM 幅值，范围为 0～MOTOR_PWM_MAX。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_Backward(uint16_t speed)
{
    int16_t clamped;

    clamped = Motor_Clamp((int16_t)speed);
    Motor_SetSpeed(-clamped, clamped);
}

/*******************************************************************************
 * 函数名称：Motor_Stop
 * 功    能：将两路 PWM 有效占空比清零，并使左右轮进入短刹车状态。
 * 参    数：无。
 * 返 回 值：无。
 ******************************************************************************/
void Motor_Stop(void)
{
    Motor_SetSpeed(0, 0);
}

/*******************************************************************************
 * 函数名称：Motor_EncoderTick
 * 功    能：保存并清零 GPIO 外部中断累计的左右轮编码器增量。
 * 调用周期：20 ms，由 TIMA1 定时中断调用。
 * 参    数：无。
 * 返 回 值：无。
 * 说    明：GPIO 编码器中断与 TIMA1 中断使用相同优先级，本函数在 TIMA1
 *           中断中执行时不会被 GPIO 中断嵌套，因此读取并清零操作不会丢计数。
 *******************************************************************************/
void Motor_EncoderTick(void)
{
    if (motor_encoder_ready == 0U) {
        return;
    }

    Motor_LeftSpeed = (int16_t)Encoder_Left_Count;
    Motor_RightSpeed = (int16_t)Encoder_Right_Count;
    Encoder_Left_Count = 0;
    Encoder_Right_Count = 0;
    Motor_LeftEncoderTotal = Get_Encoder_countA;
    Motor_RightEncoderTotal = Get_Encoder_countB;
}

#if 0
/* 旧硬件 QEI 采样实现，保留用于后续切换和对照。 */
void Motor_EncoderTick_QEI(void)
{
    int16_t leftDelta;
    int16_t rightDelta;

    leftDelta = Motor_ReadEncoderDelta(
        QEI_LEFT_INST, &motor_left_encoder_previous, 1U);
    rightDelta = Motor_ReadEncoderDelta(
        QEI_RIGHT_INST, &motor_right_encoder_previous, 0U);

    Motor_LeftSpeed = leftDelta;
    Motor_RightSpeed = rightDelta;
    Motor_LeftEncoderTotal += (int32_t)leftDelta;
    Motor_RightEncoderTotal += (int32_t)rightDelta;
}
#endif

/*******************************************************************************
 * 函数名称：Motor_GetSpeed_Left
 * 功    能：读取左轮最近一个采样周期内的有符号 QEI 增量计数。
 * 参    数：无。
 * 返 回 值：左轮 QEI 增量计数。
 *******************************************************************************/
int16_t Motor_GetSpeed_Left(void)
{
    return Motor_LeftSpeed;
}

/*******************************************************************************
 * 函数名称：Motor_GetSpeed_Right
 * 功    能：读取右轮最近一个采样周期内的有符号 QEI 增量计数。
 * 参    数：无。
 * 返 回 值：右轮 QEI 增量计数。
 *******************************************************************************/
int16_t Motor_GetSpeed_Right(void)
{
    return Motor_RightSpeed;
}

/*******************************************************************************
 * 函数名称：Motor_GetSpeedRPM_Left
 * 功    能：将左轮最近一个采样周期内的 QEI 增量换算为轮端转速。
 * 参    数：无。
 * 返 回 值：左轮轮端转速，单位为转/分。
 *******************************************************************************/
float Motor_GetSpeedRPM_Left(void)
{
    return (float)Motor_LeftSpeed * MOTOR_RPM_PER_SAMPLE_COUNT;
}

/*******************************************************************************
 * 函数名称：Motor_GetSpeedRPM_Right
 * 功    能：将右轮最近一个采样周期内的 QEI 增量换算为轮端转速。
 * 参    数：无。
 * 返 回 值：右轮轮端转速，单位为转/分。
 *******************************************************************************/
float Motor_GetSpeedRPM_Right(void)
{
    return (float)Motor_RightSpeed * MOTOR_RPM_PER_SAMPLE_COUNT;
}

/*******************************************************************************
 * 函数名称：Motor_GetEncoderTotal_Left
 * 功    能：读取左轮自初始化以来的有符号 QEI 累计计数。
 * 参    数：无。
 * 返 回 值：左轮 QEI 累计计数。
 *******************************************************************************/
int32_t Motor_GetEncoderTotal_Left(void)
{
    return Motor_LeftEncoderTotal;
}

/*******************************************************************************
 * 函数名称：Motor_GetEncoderTotal_Right
 * 功    能：读取右轮自初始化以来的有符号 QEI 累计计数。
 * 参    数：无。
 * 返 回 值：右轮 QEI 累计计数。
 *******************************************************************************/
int32_t Motor_GetEncoderTotal_Right(void)
{
    return Motor_RightEncoderTotal;
}

/*******************************************************************************
 * 函数名称：Motor_ResetEncoderTotal
 * 功    能：清零 GPIO 外部中断方式的左右轮瞬时计数和累计计数。
 * 参    数：无。
 * 返 回 值：无。
 *******************************************************************************/
void Motor_ResetEncoderTotal(void)
{
    Encoder_Left_Count = 0;
    Encoder_Right_Count = 0;
    Get_Encoder_countA = 0;
    Get_Encoder_countB = 0;
    Motor_LeftSpeed = 0;
    Motor_RightSpeed = 0;
    Motor_LeftEncoderTotal = 0;
    Motor_RightEncoderTotal = 0;
}

#if 0
/*******************************************************************************
 * 函数名称：GROUP1_IRQHandler
 * 功    能：处理左右编码器 A、B 相 GPIO 上升沿中断，并判断旋转方向。
 * 参    数：无。
 * 返 回 值：无。
 * 说    明：严格按照原编码器中断逻辑处理，每相只在上升沿计数，每个完整
 *           AB 周期产生两个计数；中断处理完成后统一清除中断标志。
 *******************************************************************************/
void GROUP1_IRQHandler(void)
{
    uint32_t gpio_interrup1;
    uint32_t gpio_interrup2;
    int32_t leftStep = 0;
    int32_t rightStep = 0;

    gpio_interrup1 = DL_GPIO_getEnabledInterruptStatus(
        ENCODERA_PORT, ENCODERA_E1A_PIN | ENCODERA_E1B_PIN);
    gpio_interrup2 = DL_GPIO_getEnabledInterruptStatus(
        ENCODERB_PORT, ENCODERB_E2A_PIN | ENCODERB_E2B_PIN);

    if ((gpio_interrup1 & ENCODERA_E1A_PIN) == ENCODERA_E1A_PIN) {
        if (!DL_GPIO_readPins(ENCODERA_PORT, ENCODERA_E1B_PIN)) {
            leftStep = -1;
        } else {
            leftStep = 1;
        }
    } else if ((gpio_interrup1 & ENCODERA_E1B_PIN) ==
        ENCODERA_E1B_PIN) {
        if (!DL_GPIO_readPins(ENCODERA_PORT, ENCODERA_E1A_PIN)) {
            leftStep = 1;
        } else {
            leftStep = -1;
        }
    }

    if (leftStep != 0) {
        Encoder_Left_Count += leftStep;
        Get_Encoder_countA += leftStep;
    }

    if ((gpio_interrup2 & ENCODERB_E2A_PIN) == ENCODERB_E2A_PIN) {
        if (!DL_GPIO_readPins(ENCODERB_PORT, ENCODERB_E2B_PIN)) {
            rightStep = -1;
        } else {
            rightStep = 1;
        }
    } else if ((gpio_interrup2 & ENCODERB_E2B_PIN) ==
        ENCODERB_E2B_PIN) {
        if (!DL_GPIO_readPins(ENCODERB_PORT, ENCODERB_E2A_PIN)) {
            rightStep = 1;
        } else {
            rightStep = -1;
        }
    }

    if (rightStep != 0) {
        Encoder_Right_Count += rightStep;
        Get_Encoder_countB += rightStep;
    }

    DL_GPIO_clearInterruptStatus(
        ENCODERA_PORT, ENCODERA_E1A_PIN | ENCODERA_E1B_PIN);
    DL_GPIO_clearInterruptStatus(
        ENCODERB_PORT, ENCODERB_E2A_PIN | ENCODERB_E2B_PIN);
}
#endif

#if 0
/* 旧硬件 QEI 复位和方向变化中断实现，保留用于后续切换。 */
void Motor_ResetEncoderTotal_QEI(void)
{
    motor_left_encoder_previous =
        (uint16_t)DL_TimerG_getTimerCount(QEI_LEFT_INST);
    motor_right_encoder_previous =
        (uint16_t)DL_TimerG_getTimerCount(QEI_RIGHT_INST);
    Motor_LeftSpeed = 0;
    Motor_RightSpeed = 0;
    Motor_LeftEncoderTotal = 0;
    Motor_RightEncoderTotal = 0;
    Motor_LeftDirectionChanges = 0U;
    Motor_RightDirectionChanges = 0U;
}

/*******************************************************************************
 * 函数名称：QEI_LEFT_INST_IRQHandler
 * 功    能：处理左轮 QEI 方向变化中断，并累计方向变化次数。
 * 参    数：无。
 * 返 回 值：无。
 * 说    明：中断配置与 TI timg_qei_mode 例程保持一致。
 *******************************************************************************/
void QEI_LEFT_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(QEI_LEFT_INST)) {
        case DL_TIMERG_IIDX_DIR_CHANGE:
            Motor_LeftDirectionChanges++;
            break;
        default:
            break;
    }
}

/*******************************************************************************
 * 函数名称：QEI_RIGHT_INST_IRQHandler
 * 功    能：处理右轮 QEI 方向变化中断，并累计方向变化次数。
 * 参    数：无。
 * 返 回 值：无。
 * 说    明：中断配置与 TI timg_qei_mode 例程保持一致。
 *******************************************************************************/
void QEI_RIGHT_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(QEI_RIGHT_INST)) {
        case DL_TIMERG_IIDX_DIR_CHANGE:
            Motor_RightDirectionChanges++;
            break;
        default:
            break;
    }
}
#endif
