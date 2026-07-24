#include "ti_msp_dl_config.h"
#include "bsp.h"

/*******************************************************************************
 * 名    称： main
 * 功    能：初始化系统，等待 USER 按键按下后执行 task1。
 * 参    数：无。
 * 出    口：int，嵌入式主循环正常不返回。
 * 说    明：等待按键期间每 5ms 更新一次 ICM45686 姿态解算。
 *******************************************************************************/
int main(void)
{
    float accel_mg[3];
    float gyro_dps[3];
    uint32_t last_update = 0U;

    SYSCFG_DL_init();
    LED_RGB_Init();
    LED_RGB_Set(0U, 0U, 0U);
    Timer_Init();

    if (!ICM45686_Init()) {
        while (1) { }
    }

    MahonyAHRS_Init();

    Motor_Init();
    Grayscale_Sensor_Init();
    Maxicam_Init();
    Key_Init();

    while (1) {
        if ((uint32_t)(nowtime - last_update) >= 5U) {
            last_update = nowtime;
            ICM45686_ReadAccelGyro(accel_mg, gyro_dps);
            MahonyAHRS_Update(gyro_dps[0], gyro_dps[1], gyro_dps[2],
                              accel_mg[0], accel_mg[1], accel_mg[2]);
        }

        if (Key_GetNum() == KEY_NUM_USER) {
            task1();
            last_update = nowtime;
        }
    }
}
