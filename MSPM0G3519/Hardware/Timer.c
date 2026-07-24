#include "Timer.h"
#include "Key.h"
#include "Motor.h"
#include "ti_msp_dl_config.h"

volatile uint32_t nowtime = 0;

/*******************************************************************************
 * 名    称： Timer_Init
 * 功    能：使能 TIMER_Clock 中断，为系统提供 1ms 软件计时基准。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void Timer_Init(void)
{
    NVIC_ClearPendingIRQ(TIMER_Clock_INST_INT_IRQN);
    NVIC_EnableIRQ(TIMER_Clock_INST_INT_IRQN);
}

/*******************************************************************************
 * 名    称： TIMER_Clock_INST_IRQHandler
 * 功    能：处理 TIMA1 的 100 微秒周期中断，软件分频生成 1 毫秒系统时基、
 *           10 毫秒按键扫描节拍和 20 毫秒编码器速度采样节拍。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void TIMER_Clock_INST_IRQHandler(void)
{
    static uint8_t millisecondDivider = 0U;
    static uint8_t keyTickDivider = 0U;
    static uint8_t motorTickDivider = 0U;

    switch (DL_TimerA_getPendingInterrupt(TIMER_Clock_INST)) {
        case DL_TIMERA_IIDX_LOAD:
            millisecondDivider++;
            if (millisecondDivider >= 10U) {
                millisecondDivider = 0U;
                nowtime++;

                keyTickDivider++;
                if (keyTickDivider >= KEY_TICK_PERIOD_MS) {
                    keyTickDivider = 0U;
                    Key_Tick();
                }

                motorTickDivider++;
                if (motorTickDivider >= MOTOR_ENCODER_SAMPLE_PERIOD_MS) {
                    motorTickDivider = 0U;
                    Motor_EncoderTick();
                }
            }
            break;
        default:
            break;
    }
}
