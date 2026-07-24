#ifndef LED_H
#define LED_H

#include <stdint.h>

/*******************************************************************************
 * WS2812 RGB 灯接口
 * 说    明：使用 SysConfig 配置的 TIMG6（PA29）发送数据，颜色参数顺序
 *           固定为 G、R、B。
 *******************************************************************************/
void LED_RGB_Init(void);
void LED_RGB_Set(uint8_t g, uint8_t r, uint8_t b);

/*******************************************************************************
 * 板载指示灯接口
 * 说    明：L1 和 L2 均为低电平有效。
 *******************************************************************************/
void LED_L1_On(void);
void LED_L1_Off(void);
void LED_L1_Toggle(void);
void LED_L2_On(void);
void LED_L2_Off(void);
void LED_L2_Toggle(void);
void LED_Alternate(void);

#endif
