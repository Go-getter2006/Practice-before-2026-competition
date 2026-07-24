#include "Ultrasonic.h"
#include "Delay.h"
#include "ti_msp_dl_config.h"

/* 捕获定时器当前配置为一兆赫兹，因此一个计数值对应一微秒。 */
#define ULTRASONIC_CAPTURE_FREQUENCY_HZ    (1000000UL)

/* 常温下声速按照每秒三万四千三百厘米计算。 */
#define ULTRASONIC_SOUND_SPEED_CM_S        (34300.0f)

/* 触发端保持高电平十微秒，以启动超声波模块。 */
#define ULTRASONIC_TRIGGER_HIGH_US         (10U)

/* 触发前先保持短暂低电平，避免残留高电平造成误触发。 */
#define ULTRASONIC_TRIGGER_LOW_US          (2U)

/* 捕获周期为六十毫秒，超过该时间仍无回波则判定本次测量超时。 */
#define ULTRASONIC_TIMEOUT_US              (60000UL)

/* 等待过程中每十微秒检查一次捕获完成标志。 */
#define ULTRASONIC_POLL_STEP_US            (10U)

/* 保存中断中获得的回波高电平宽度，单位为定时器计数值。 */
static volatile uint32_t s_echoPulseTicks = 0U;

/* 标记当前是否正在等待本次测量的回波。 */
static volatile uint8_t s_waitingForEcho = 0U;

/* 标记本次回波脉宽是否已经捕获完成。 */
static volatile uint8_t s_captureFinished = 0U;

/*
 * 功能：初始化超声波测距驱动。
 * 说明：系统配置已经完成引脚复用和捕获定时器初始化，本函数负责清除状态并开启中断。
 */
void Ultrasonic_Init(void)
{
    DL_GPIO_clearPins(WAVE_PORT, WAVE_Trig_PIN);

    s_echoPulseTicks = 0U;
    s_waitingForEcho = 0U;
    s_captureFinished = 0U;

    DL_TimerG_clearInterruptStatus(
        CAPTURE_WAVE_INST, DL_TIMERG_INTERRUPT_CC3_UP_EVENT);
    NVIC_ClearPendingIRQ(CAPTURE_WAVE_INST_INT_IRQN);
    NVIC_EnableIRQ(CAPTURE_WAVE_INST_INT_IRQN);
}

/*
 * 功能：触发一次超声波测距，并将结果换算为厘米。
 * 参数：距离结果指针用于保存测量结果。
 * 返回：成功获得回波返回一，否则返回零。
 */
uint8_t Ultrasonic_MeasureDistanceCm(float *distanceCm)
{
    uint32_t waitCount;
    uint32_t totalWaitCount;
    uint32_t capturedTicks;
    uint8_t measurementSucceeded;

    if (distanceCm == (void *) 0) {
        return 0U;
    }

    *distanceCm = 0.0f;
    s_waitingForEcho = 0U;

    DL_TimerG_clearInterruptStatus(
        CAPTURE_WAVE_INST, DL_TIMERG_INTERRUPT_CC3_UP_EVENT);
    NVIC_ClearPendingIRQ(CAPTURE_WAVE_INST_INT_IRQN);

    s_captureFinished = 0U;
    s_echoPulseTicks = 0U;
    s_waitingForEcho = 1U;

    DL_GPIO_clearPins(WAVE_PORT, WAVE_Trig_PIN);
    Delay_us(ULTRASONIC_TRIGGER_LOW_US);
    DL_GPIO_setPins(WAVE_PORT, WAVE_Trig_PIN);
    Delay_us(ULTRASONIC_TRIGGER_HIGH_US);
    DL_GPIO_clearPins(WAVE_PORT, WAVE_Trig_PIN);

    totalWaitCount = ULTRASONIC_TIMEOUT_US / ULTRASONIC_POLL_STEP_US;
    measurementSucceeded = 0U;

    for (waitCount = 0U; waitCount < totalWaitCount; waitCount++) {
        if (s_captureFinished != 0U) {
            measurementSucceeded = 1U;
            break;
        }

        Delay_us(ULTRASONIC_POLL_STEP_US);
    }

    s_waitingForEcho = 0U;

    if (measurementSucceeded != 0U) {
        capturedTicks = s_echoPulseTicks;
        if ((capturedTicks > 0U) &&
            (capturedTicks <= CAPTURE_WAVE_INST_LOAD_VALUE)) {
            *distanceCm =
                ((float) capturedTicks * ULTRASONIC_SOUND_SPEED_CM_S) /
                (2.0f * (float) ULTRASONIC_CAPTURE_FREQUENCY_HZ);
        } else {
            measurementSucceeded = 0U;
        }
    }

    /* 即使提前收到回波，也继续补足六十毫秒测量周期，避免连续触发相互干扰。 */
    while (waitCount < totalWaitCount) {
        Delay_us(ULTRASONIC_POLL_STEP_US);
        waitCount++;
    }

    return measurementSucceeded;
}

/*
 * 功能：处理捕获定时器中断，并保存回波高电平持续时间。
 * 说明：上升沿由硬件开始计时，下降沿由硬件锁存计数值并产生本中断。
 */
void CAPTURE_WAVE_INST_IRQHandler(void)
{
    switch (DL_TimerG_getPendingInterrupt(CAPTURE_WAVE_INST)) {
        case DL_TIMER_IIDX_CC3_UP:
            if (s_waitingForEcho != 0U) {
                s_echoPulseTicks = DL_TimerG_getCaptureCompareValue(
                    CAPTURE_WAVE_INST, DL_TIMER_CC_3_INDEX);
                s_waitingForEcho = 0U;
                s_captureFinished = 1U;
            }
            break;

        default:
            break;
    }
}
