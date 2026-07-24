#include "Greyscale.h"
#include "Delay.h"
#include "ti_msp_dl_config.h"

/*******************************************************************************
 * 全局巡线误差。
 * 说    明：初始值为 0，无法识别灰度状态时保持上一次有效值。
 ******************************************************************************/
float error = 0.0f;

/*******************************************************************************
 * 名    称： Grayscale_Select_Channel
 * 功    能：通过 AD0、AD1、AD2 三根选择线切换灰度传感器通道。
 * 说    明：AD0 对应通道编号最低位，AD2 对应最高位，通道 0～7
 *           分别对应二进制 000～111。
 * 参    数：channel：灰度传感器通道编号，范围为 0～7。
 * 出    口：无返回值。
 ******************************************************************************/
static void Grayscale_Select_Channel(uint8_t channel)
{
    DL_GPIO_clearPins(GREY_PORT, GREY_AD0_PIN | GREY_AD1_PIN | GREY_AD2_PIN);

    if ((channel & 0x01U) != 0U) {
        DL_GPIO_setPins(GREY_PORT, GREY_AD0_PIN);
    }
    if ((channel & 0x02U) != 0U) {
        DL_GPIO_setPins(GREY_PORT, GREY_AD1_PIN);
    }
    if ((channel & 0x04U) != 0U) {
        DL_GPIO_setPins(GREY_PORT, GREY_AD2_PIN);
    }
}

/*******************************************************************************
 * 名    称： Grayscale_Read_OUT_Value
 * 功    能：读取灰度传感器 OUT 引脚的当前电平。
 * 参    数：无。
 * 出    口：高电平返回 1，低电平返回 0。
 ******************************************************************************/
static uint16_t Grayscale_Read_OUT_Value(void)
{
    return (DL_GPIO_readPins(GREY_PORT, GREY_OUT_PIN) != 0U) ? 1U : 0U;
}

/*******************************************************************************
 * 名    称： Grayscale_Sensor_Init
 * 功    能：初始化灰度传感器的通道选择状态。
 * 说    明：GPIO 方向由 SysConfig 配置，此处将 AD0、AD1、AD2 全部置为
 *           低电平，默认选择通道 0。
 * 参    数：无。
 * 出    口：无返回值。
 ******************************************************************************/
void Grayscale_Sensor_Init(void)
{
    Grayscale_Select_Channel(0U);
}

/*******************************************************************************
 * 名    称： Grayscale_Sensor_Read_All
 * 功    能：依次轮询八个通道，并读取每个通道 OUT 引脚的高低电平。
 * 说    明：每次切换通道后延时 100 us，等待模拟开关输出稳定；读取结果
 *           按通道编号依次保存，高电平保存为 1，低电平保存为 0。
 * 参    数：sensor_values：至少包含 8 个 uint16_t 元素的数组。
 * 出    口：无返回值；sensor_values 为空指针时不执行读取。
 ******************************************************************************/
void Grayscale_Sensor_Read_All(uint16_t* sensor_values)
{
    uint8_t i;

    if (sensor_values == 0) {
        return;
    }

    for (i = 0U; i < GRAYSCALE_SENSOR_CHANNELS; i++) {
        Grayscale_Select_Channel(i);
        Delay_us(100U);
        sensor_values[i] = Grayscale_Read_OUT_Value();
    }
}

/*******************************************************************************
 * 名    称： Grayscale_Sensor_Read_Single
 * 功    能：选择并读取指定灰度传感器通道的 OUT 引脚电平。
 * 说    明：切换通道后延时 50 us，等待模拟开关输出稳定。
 * 参    数：channel：灰度传感器通道编号，范围为 0～7。
 * 出    口：高电平返回 1，低电平或通道编号无效时返回 0。
 ******************************************************************************/
uint16_t Grayscale_Sensor_Read_Single(uint8_t channel)
{
    if (channel >= GRAYSCALE_SENSOR_CHANNELS) {
        return 0U;
    }

    Grayscale_Select_Channel(channel);
    Delay_us(50U);
    return Grayscale_Read_OUT_Value();
}

/*******************************************************************************
 * 名    称： Get_Grayscale_State
 * 功    能：读取八路灰度传感器，并将各通道电平组合为 8 位状态值。
 * 说    明：通道 0 对应 bit7，通道 7 对应 bit0；OUT 为高电平时将对应
 *           状态位置 1，OUT 为低电平时保持为 0。
 * 参    数：无。
 * 出    口：八路灰度传感器的组合状态。
 ******************************************************************************/
uint8_t Get_Grayscale_State(void)
{
    uint16_t sensor_values[GRAYSCALE_SENSOR_CHANNELS];
    uint8_t state = 0U;
    uint8_t i;

    Grayscale_Sensor_Read_All(sensor_values);

    for (i = 0U; i < GRAYSCALE_SENSOR_CHANNELS; i++) {
        if (sensor_values[i] != 0U) {
            state |= (uint8_t)(1U << (7U - i));
        }
    }

    return state;
}

/*******************************************************************************
 * 名    称： Track_err
 * 功    能：根据八路灰度传感器状态计算巡线误差。
 * 说    明：沿用原 STM32 巡线程序中的状态与误差对应关系；当前状态没有
 *           对应误差时保持上一次结果，避免无效跳变导致误差突然归零。
 * 参    数：state：八路灰度传感器的组合状态。
 * 出    口：当前巡线误差；正值表示右偏，负值表示左偏。
 ******************************************************************************/
float Track_err(uint8_t state)
{
    switch (state) {
        case 0x00U:
        case 0x18U:
        case 0x3CU:
        case 0x7EU:
            error = 0.0f;
            break;

        case 0x10U:
        case 0x30U:
        case 0x38U:
            error = 20.0f;
            break;

        case 0x20U:
            error = 30.0f;
            break;

        case 0x40U:
        case 0x60U:
            error = 40.0f;
            break;

        case 0x80U:
        case 0xC0U:
            error = 60.0f;
            break;

        case 0x08U:
        case 0x0CU:
        case 0x1CU:
            error = -20.0f;
            break;

        case 0x04U:
            error = -30.0f;
            break;

        case 0x02U:
        case 0x06U:
            error = -40.0f;
            break;

        case 0x01U:
        case 0x03U:
            error = -60.0f;
            break;

        default:
            break;
    }

    return error;
}
