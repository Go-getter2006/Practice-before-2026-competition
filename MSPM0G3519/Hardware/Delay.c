#include "Delay.h"
#include "ti_msp_dl_config.h"

#define DELAY_US_CYCLES    (CPUCLK_FREQ / 1000000U)
#define DELAY_MS_CYCLES    (CPUCLK_FREQ / 1000U)

/*
 * 功能：微秒级阻塞延时。
 * 参数：us 表示需要延时的微秒数。
 * 输出：无返回值，延时结束后继续执行。
 */
void Delay_us(uint32_t us)
{
    while (us > 0U) {
        delay_cycles(DELAY_US_CYCLES);
        us--;
    }
}

/*
 * 功能：毫秒级阻塞延时。
 * 参数：ms 表示需要延时的毫秒数。
 * 输出：无返回值，延时结束后继续执行。
 */
void Delay_ms(uint32_t ms)
{
    while (ms > 0U) {
        delay_cycles(DELAY_MS_CYCLES);
        ms--;
    }
}
