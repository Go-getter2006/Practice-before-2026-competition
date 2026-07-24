#include "Delay.h"
#include "stm32f4xx_hal.h"

static uint32_t g_fac_us = 0;   // 1微秒所需的 SysTick 计数次数

/**
  * @brief 延时初始化，计算时钟系数
  */
void Delay_Init(void)
{
    // 获取 HCLK 时钟频率（Hz），计算 1us 所需的 SysTick 计数次数
    // SysTick 的计数频率等于 HCLK（在 HAL 库中默认如此）
    g_fac_us = HAL_RCC_GetHCLKFreq() / 1000000;
}

/**
  * @brief 微秒延时（时钟摘取法）
  * @param nus 延时微秒数
  */
void Delay_us(uint32_t nus)
{
    uint32_t ticks;          // 需要等待的总计数
    uint32_t told, tnow;     // 前后两次读取的 VAL 值
    uint32_t tcnt = 0;       // 已累计的计数
    uint32_t reload;         // SysTick 重装载值

    if (g_fac_us == 0)       // 防止未调用初始化
        Delay_Init();

    reload = SysTick->LOAD;                  // 获取重装载值
    ticks = nus * g_fac_us;                   // 需要等待的计数总数
    told = SysTick->VAL;                      // 读取当前计数值

    while (1)
    {
        tnow = SysTick->VAL;                   // 读取新的计数值
        if (tnow != told)
        {
            // 计算两次读取之间的计数差
            if (tnow < told)
            {
                // 没有溢出，差值 = told - tnow
                tcnt += told - tnow;
            }
            else
            {
                // 发生了溢出（重新从 LOAD 开始递减）
                tcnt += reload - tnow + told;
            }
            told = tnow;                        // 更新参考值

            if (tcnt >= ticks)                   // 达到或超过目标延时
                break;
        }
    }
}

/**
  * @brief 毫秒延时（多次调用微秒延时）
  */
void Delay_ms(uint32_t nms)
{
    while (nms--)
    {
        Delay_us(1000);
    }
}

/**
  * @brief 秒延时
  */
void Delay_s(uint32_t ns)
{
    while (ns--)
    {
        Delay_ms(1000);
    }
}
