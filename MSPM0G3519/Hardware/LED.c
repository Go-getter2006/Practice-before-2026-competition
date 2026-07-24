#include "LED.h"
#include "ti_msp_dl_config.h"
#include <stdint.h>

/*
 * TIMG6时钟80 MHz，周期计数100，对应WS2812的800 kHz时序。
 * 参数顺序保持原有的G、R、B，编码顺序为GRB、MSB优先。
 */
#define RGB_BIT_COUNT             (24U)
#define RGB_RESET_PERIOD_COUNT    (75U)
#define RGB_DMA_TRANSFER_COUNT    \
    (RGB_RESET_PERIOD_COUNT + RGB_BIT_COUNT + 1U)
#define RGB_BIT_0_COMPARE         (75U)
#define RGB_BIT_1_COMPARE         (50U)
#define RGB_RESET_COMPARE         (99U)

/*
 * DMA依次发送75个复位周期、24位GRB数据和1个结束复位周期。
 * 缓冲区不使用volatile；DMA启动前CPU已完成全部写入。
 */
static uint32_t RGB_dmaData[RGB_DMA_TRANSFER_COUNT];

/*******************************************************************************
 * 名    称：LED_RGB_Init
 * 功    能：初始化WS2812的DMA发送环境，并让TIMG6保持复位波形。
 * 说    明：TIMG6只向DMA发布LOAD事件，不再产生1.25 us的CPU中断。
 *******************************************************************************/
void LED_RGB_Init(void)
{
    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
    DL_DMA_clearInterruptStatus(DMA, DL_DMA_INTERRUPT_CHANNEL0);

    DL_TimerG_setCaptureCompareValue(
        PWM_RGB_INST, RGB_RESET_COMPARE, DL_TIMER_CC_0_INDEX);

    NVIC_DisableIRQ(PWM_RGB_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(PWM_RGB_INST_INT_IRQN);
    NVIC_ClearPendingIRQ(DMA_INT_IRQn);
    NVIC_EnableIRQ(DMA_INT_IRQn);

    DL_TimerG_startCounter(PWM_RGB_INST);
}

/*******************************************************************************
 * 名    称：LED_RGB_Set
 * 功    能：按原有(G,R,B)参数顺序生成WS2812波形，并启动一次DMA发送。
 * 说    明：DMA每收到一次TIMG6 LOAD事件只搬运一个32位比较值。
 *******************************************************************************/
void LED_RGB_Set(uint8_t G, uint8_t R, uint8_t B)
{
    const uint8_t color[3] = {G, R, B};
    uint32_t index;
    uint8_t channel;
    uint8_t bit;

    DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);

    for (index = 0U; index < RGB_RESET_PERIOD_COUNT; index++) {
        RGB_dmaData[index] = RGB_RESET_COMPARE;
    }

    for (channel = 0U; channel < 3U; channel++) {
        for (bit = 0U; bit < 8U; bit++) {
            index = RGB_RESET_PERIOD_COUNT +
                    ((uint32_t)channel * 8U) + (uint32_t)bit;
            RGB_dmaData[index] =
                ((color[channel] & (uint8_t)(0x80U >> bit)) != 0U) ?
                RGB_BIT_1_COMPARE : RGB_BIT_0_COMPARE;
        }
    }

    RGB_dmaData[RGB_DMA_TRANSFER_COUNT - 1U] = RGB_RESET_COMPARE;

    DL_DMA_clearInterruptStatus(DMA, DL_DMA_INTERRUPT_CHANNEL0);
    NVIC_ClearPendingIRQ(DMA_INT_IRQn);
    DL_DMA_setSrcAddr(
        DMA, DMA_CH0_CHAN_ID, (uint32_t)(uintptr_t)&RGB_dmaData[0]);
    DL_DMA_setDestAddr(
        DMA,
        DMA_CH0_CHAN_ID,
        (uint32_t)(uintptr_t)&PWM_RGB_INST->COUNTERREGS.CC_01[0]);
    DL_DMA_setTransferSize(
        DMA, DMA_CH0_CHAN_ID, (uint16_t)RGB_DMA_TRANSFER_COUNT);
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
}

/*******************************************************************************
 * 名    称：DMA_IRQHandler
 * 功    能：处理RGB DMA整帧发送完成。
 * 说    明：每次LED_RGB_Set仅进入一次该中断，结束后CC0保持复位比较值。
 *******************************************************************************/
void DMA_IRQHandler(void)
{
    if (DL_DMA_getPendingInterrupt(DMA) == DL_DMA_EVENT_IIDX_DMACH0) {
        DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
        DL_TimerG_setCaptureCompareValue(
            PWM_RGB_INST, RGB_RESET_COMPARE, DL_TIMER_CC_0_INDEX);
    }
}

/*******************************************************************************
 * 板载指示灯接口，L1和L2均为低电平有效。
 *******************************************************************************/
void LED_L1_On(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_L1_PIN);
}

void LED_L1_Off(void)
{
    DL_GPIO_setPins(LED_PORT, LED_L1_PIN);
}

void LED_L1_Toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_L1_PIN);
}

void LED_L2_On(void)
{
    DL_GPIO_clearPins(LED_PORT, LED_L2_PIN);
}

void LED_L2_Off(void)
{
    DL_GPIO_setPins(LED_PORT, LED_L2_PIN);
}

void LED_L2_Toggle(void)
{
    DL_GPIO_togglePins(LED_PORT, LED_L2_PIN);
}

void LED_Alternate(void)
{
    static uint8_t current = 0U;

    if (current == 0U) {
        LED_L1_On();
        LED_L2_Off();
        current = 1U;
    } else {
        LED_L1_Off();
        LED_L2_On();
        current = 0U;
    }
}
