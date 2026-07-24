#ifndef ICM45686_H
#define ICM45686_H

#include <stdint.h>

/*******************************************************************************
 * ICM45686 备用 IMU 模块接入说明
 *
 * 一、SysConfig 图形化配置：SPI_IMU
 *   1. 在 SPI 中新增 1 个实例，名称必须填写为：SPI_IMU。
 *   2. 顶部配置：
 *        Name                = SPI_IMU
 *        Selected Peripheral = SPI1
 *        SPI Profiles        = Custom
 *   3. Basic Configuration / SPI Initialization Configuration：
 *        Mode Select          = Controller
 *        Target Bit Rate (Hz) = 20000000
 *        Frame Format         = Motorola 3-wire
 *        Clock Polarity       = High
 *        Phase                = Data captured on second clock edge
 *        Frame Size (bits)    = 8
 *        Bit Order            = MSB
 *   4. PinMux / Peripheral and Pin Configuration：
 *        SPI Peripheral                          = SPI1
 *        SPI SCLK (Clock)                        = PB16/40
 *        SPI PICO (Peripheral In, Controller Out)= PB15/39
 *        SPI POCI (Peripheral Out, Controller In)= PB14/38
 *
 * 二、SysConfig 图形化配置：IMU 片选 GPIO
 *   1. 在 GPIO 中新增 1 个实例，名称必须填写为：IMU。
 *   2. 在 Group Pins 中新增 1 个引脚，名称必须填写为：CS_IMU。
 *   3. CS_IMU 配置：
 *        Name                   = CS_IMU
 *        Direction              = Output
 *        Initial Value          = Set
 *        IO Structure           = Any
 *        Internal Resistor      = Pull-Up Resistor
 *        Invert                 = Disabled
 *        Drive Strength Control = LOW
 *        High-Impedance         = Disabled
 *        PinMux / CS_IMU        = PB12/36
 *   4. CS_IMU 为低电平有效：空闲时保持 Set，高电平释放；通信时由驱动拉低选中。
 *
 * 三、生成后需要检查的关键名称
 *   1. User/ti_msp_dl_config.c 中应生成：
 *        .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1
 *        DL_SPI_setBitRateSerialClockDivider(SPI_IMU_INST, 1)
 *   2. User/ti_msp_dl_config.h 中应生成：
 *        #define SPI_IMU_INST        SPI1
 *        #define IMU_PORT            (GPIOB)
 *        #define IMU_CS_IMU_PIN      (DL_GPIO_PIN_12)
 *   3. 如果重新生成后这些名称变化，本驱动中的 SPI_IMU_INST、IMU_PORT、
 *      IMU_CS_IMU_PIN 就会失效，因此 SysConfig 中的 Name 不要改。
 *
 * 四、BSP/bsp.h 中用户自定义头文件区域需要包含
 *        #include "Delay.h"
 *        #include "LED.h"
 *        #include "UART.h"
 *        #include "Timer.h"
 *        #include "ICM45686.h"
 *        #include "MahonyAHRS.h"
 *
 * 五、User/main.c 完整接入结构
 *   1. 文件头部只需要包含：
 *        #include "ti_msp_dl_config.h"
 *        #include "bsp.h"
 *
 *   2. main() 前需要先写 UART0_SendEulerAngles()。这是 main.c 内部使用的
 *      static 辅助函数，用于把 MahonyAHRS_GetYawPitchRoll() 得到的 ypr[3]
 *      按 YAW/PITCH/ROLL 格式输出到 UART0。这里使用 UART0_SendFloat2()
 *      手写两位小数输出，避免 sprintf("%.2f") 占用较多栈空间。
 *
 *        static void UART0_SendEulerAngles(float ypr[3])
 *        {
 *            UART0_SendString("YAW=");
 *            UART0_SendFloat2(ypr[0]);
 *            UART0_SendString(" PITCH=");
 *            UART0_SendFloat2(ypr[1]);
 *            UART0_SendString(" ROLL=");
 *            UART0_SendFloat2(ypr[2]);
 *            UART0_SendString("\r\n");
 *        }
 *
 *   3. main() 中变量名称保持如下，不要随意改名：
 *        float accel_mg[3];
 *        float gyro_dps[3];
 *        float ypr[3];
 *        uint32_t last_update = 0;
 *        uint32_t last_print  = 0;
 *
 *   4. main() 初始化顺序保持如下：
 *        SYSCFG_DL_init();
 *        UART0_Init();
 *        Timer_Init();
 *
 *        if (!ICM45686_Init()) {
 *            UART0_SendString("ICM45686 init failed!\r\n");
 *            while (1) { }
 *        }
 *
 *        MahonyAHRS_Init();
 *
 *   5. main() 主循环中每 5ms 读取一次 IMU，并把加速度/角速度送入
 *      Mahony 姿态解算。这里依赖 Timer.c 中断维护的 nowtime。
 *
 *        if ((uint32_t)(nowtime - last_update) >= 5U) {
 *            last_update = nowtime;
 *            ICM45686_ReadAccelGyro(accel_mg, gyro_dps);
 *            MahonyAHRS_Update(gyro_dps[0], gyro_dps[1], gyro_dps[2],
 *                              accel_mg[0], accel_mg[1], accel_mg[2]);
 *        }
 *
 *   6. main() 主循环中每 50ms 输出一次欧拉角。ypr[0] 为 Yaw，
 *      ypr[1] 为 Pitch，ypr[2] 为 Roll，单位为度。
 *
 *        if ((uint32_t)(nowtime - last_print) >= 50U) {
 *            last_print = nowtime;
 *            MahonyAHRS_GetYawPitchRoll(ypr);
 *            UART0_SendEulerAngles(ypr);
 *        }
 *
 *   7. 若只想验证 ICM45686 通信，不需要 Mahony 姿态解算，可只调用
 *      ICM45686_Init() 和 ICM45686_ReadAccelGyro()；若要输出姿态角，则
 *      UART0_SendEulerAngles()、Timer_Init()、MahonyAHRS_Init() 和上面的
 *      5ms/50ms 周期逻辑都需要保留。
 *
 * 六、依赖项说明
 *   1. Timer 模块需要 SysConfig 中存在 TIMER_Clock，Timer.c 提供 nowtime。
 *   2. UART0 调试输出需要 SysConfig 中存在 UART_0，UART.c 提供 UART0_SendString()
 *      和 UART0_SendFloat2()。
 *   3. 后续加入多路 UART、编码器电机、蜂鸣器、CCD 或 8 路灰度后，
 *      Project/startup_mspm0g351x_uvision.s 中
 *      Stack_Size 建议至少 0x00001000。大数组、串口接收缓冲区和 CCD/灰度
 *      数据缓冲区不要定义成函数内局部变量，建议定义为 static 或全局变量。
 *   4. SysConfig 相关配置应通过图形化界面修改后重新生成，不建议手改
 *      User/config.syscfg 或 User/ti_msp_dl_config.c。
 *******************************************************************************/

/* WHO_AM_I 寄存器地址及期望值，用于判断芯片是否在线。 */
#define ICM45686_REG_WHO_AM_I       (0x72U)
#define ICM45686_WHOAMI_VALUE       (0xE9U)

/*******************************************************************************
 * 名    称： ICM45686_Init
 * 功    能：初始化 ICM45686，完成 WHO_AM_I 校验、软复位和量程/输出速率配置。
 * 参    数：无。
 * 出    口：uint8_t，1 表示初始化成功，0 表示通信或芯片状态异常。
 *******************************************************************************/
uint8_t ICM45686_Init(void);

/*******************************************************************************
 * 名    称： ICM45686_CheckWhoAmI
 * 功    能：读取 WHO_AM_I 寄存器并与期望值比较，用于单独确认 IMU 是否在线。
 * 参    数：无。
 * 出    口：uint8_t，1 表示校验通过，0 表示校验失败。
 *******************************************************************************/
uint8_t ICM45686_CheckWhoAmI(void);

/*******************************************************************************
 * 名    称： ICM45686_ReadAccel
 * 功    能：读取 X/Y/Z 三轴加速度原始值，并换算为 mg 单位。
 * 参    数：accel_mg：长度为 3 的数组，用于保存 X/Y/Z 三轴加速度。
 * 出    口：无返回值，读取结果通过 accel_mg 输出。
 *******************************************************************************/
void ICM45686_ReadAccel(float accel_mg[3]);

/*******************************************************************************
 * 名    称： ICM45686_ReadAccelGyro
 * 功    能：连续读取三轴加速度和三轴陀螺仪数据，并完成单位换算。
 * 参    数：accel_mg 保存加速度 mg 值；gyro_dps 保存角速度 dps 值。
 * 出    口：无返回值，读取结果通过两个数组输出。
 *******************************************************************************/
void ICM45686_ReadAccelGyro(float accel_mg[3], float gyro_dps[3]);

#endif
