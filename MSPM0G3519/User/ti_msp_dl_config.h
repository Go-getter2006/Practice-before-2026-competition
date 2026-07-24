/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G351X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G351X
#define CONFIG_MSPM0G3519

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define GPIO_LFXT_PORT                                                     GPIOA
#define GPIO_LFXIN_PIN                                             DL_GPIO_PIN_3
#define GPIO_LFXIN_IOMUX                                          (IOMUX_PINCM8)
#define GPIO_LFXOUT_PIN                                            DL_GPIO_PIN_4
#define GPIO_LFXOUT_IOMUX                                         (IOMUX_PINCM9)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for PWM_Motor */
#define PWM_Motor_INST                                                     TIMA0
#define PWM_Motor_INST_IRQHandler                               TIMA0_IRQHandler
#define PWM_Motor_INST_INT_IRQN                                 (TIMA0_INT_IRQn)
#define PWM_Motor_INST_CLK_FREQ                                         80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_Motor_C0_PORT                                             GPIOC
#define GPIO_PWM_Motor_C0_PIN                                      DL_GPIO_PIN_2
#define GPIO_PWM_Motor_C0_IOMUX                                  (IOMUX_PINCM76)
#define GPIO_PWM_Motor_C0_IOMUX_FUNC                 IOMUX_PINCM76_PF_TIMA0_CCP0
#define GPIO_PWM_Motor_C0_IDX                                DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_Motor_C1_PORT                                             GPIOC
#define GPIO_PWM_Motor_C1_PIN                                      DL_GPIO_PIN_4
#define GPIO_PWM_Motor_C1_IOMUX                                  (IOMUX_PINCM78)
#define GPIO_PWM_Motor_C1_IOMUX_FUNC                 IOMUX_PINCM78_PF_TIMA0_CCP1
#define GPIO_PWM_Motor_C1_IDX                                DL_TIMER_CC_1_INDEX

/* Defines for PWM_RGB */
#define PWM_RGB_INST                                                       TIMG6
#define PWM_RGB_INST_IRQHandler                                 TIMG6_IRQHandler
#define PWM_RGB_INST_INT_IRQN                                   (TIMG6_INT_IRQn)
#define PWM_RGB_INST_CLK_FREQ                                           80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_RGB_C0_PORT                                               GPIOA
#define GPIO_PWM_RGB_C0_PIN                                       DL_GPIO_PIN_29
#define GPIO_PWM_RGB_C0_IOMUX                                     (IOMUX_PINCM4)
#define GPIO_PWM_RGB_C0_IOMUX_FUNC                    IOMUX_PINCM4_PF_TIMG6_CCP0
#define GPIO_PWM_RGB_C0_IDX                                  DL_TIMER_CC_0_INDEX

/* Publisher defines */
#define PWM_RGB_INST_PUB_0_CH                                                (1)



/* Defines for CAPTURE_WAVE */
#define CAPTURE_WAVE_INST                                               (TIMG14)
#define CAPTURE_WAVE_INST_IRQHandler                           TIMG14_IRQHandler
#define CAPTURE_WAVE_INST_INT_IRQN                             (TIMG14_INT_IRQn)
#define CAPTURE_WAVE_INST_LOAD_VALUE                                    (59999U)
/* GPIO defines for channel 3 */
#define GPIO_CAPTURE_WAVE_C3_PORT                                          GPIOC
#define GPIO_CAPTURE_WAVE_C3_PIN                                   DL_GPIO_PIN_5
#define GPIO_CAPTURE_WAVE_C3_IOMUX                               (IOMUX_PINCM79)
#define GPIO_CAPTURE_WAVE_C3_IOMUX_FUNC             IOMUX_PINCM79_PF_TIMG14_CCP3





/* Defines for TIMER_Clock */
#define TIMER_Clock_INST                                                 (TIMA1)
#define TIMER_Clock_INST_IRQHandler                             TIMA1_IRQHandler
#define TIMER_Clock_INST_INT_IRQN                               (TIMA1_INT_IRQn)
#define TIMER_Clock_INST_LOAD_VALUE                                         (9U)



/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                           40000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_40_MHZ_115200_BAUD                                      (21)
#define UART_0_FBRD_40_MHZ_115200_BAUD                                      (45)
/* Defines for UART_Screen */
#define UART_Screen_INST                                                   UART1
#define UART_Screen_INST_FREQUENCY                                      40000000
#define UART_Screen_INST_IRQHandler                             UART1_IRQHandler
#define UART_Screen_INST_INT_IRQN                                 UART1_INT_IRQn
#define GPIO_UART_Screen_RX_PORT                                           GPIOB
#define GPIO_UART_Screen_TX_PORT                                           GPIOB
#define GPIO_UART_Screen_RX_PIN                                    DL_GPIO_PIN_5
#define GPIO_UART_Screen_TX_PIN                                    DL_GPIO_PIN_4
#define GPIO_UART_Screen_IOMUX_RX                                (IOMUX_PINCM18)
#define GPIO_UART_Screen_IOMUX_TX                                (IOMUX_PINCM17)
#define GPIO_UART_Screen_IOMUX_RX_FUNC                 IOMUX_PINCM18_PF_UART1_RX
#define GPIO_UART_Screen_IOMUX_TX_FUNC                 IOMUX_PINCM17_PF_UART1_TX
#define UART_Screen_BAUD_RATE                                           (115200)
#define UART_Screen_IBRD_40_MHZ_115200_BAUD                                 (21)
#define UART_Screen_FBRD_40_MHZ_115200_BAUD                                 (45)
/* Defines for UART_Machine */
#define UART_Machine_INST                                                  UART3
#define UART_Machine_INST_FREQUENCY                                     80000000
#define UART_Machine_INST_IRQHandler                            UART3_IRQHandler
#define UART_Machine_INST_INT_IRQN                                UART3_INT_IRQn
#define GPIO_UART_Machine_RX_PORT                                          GPIOA
#define GPIO_UART_Machine_TX_PORT                                          GPIOA
#define GPIO_UART_Machine_RX_PIN                                  DL_GPIO_PIN_26
#define GPIO_UART_Machine_TX_PIN                                  DL_GPIO_PIN_25
#define GPIO_UART_Machine_IOMUX_RX                               (IOMUX_PINCM59)
#define GPIO_UART_Machine_IOMUX_TX                               (IOMUX_PINCM55)
#define GPIO_UART_Machine_IOMUX_RX_FUNC                IOMUX_PINCM59_PF_UART3_RX
#define GPIO_UART_Machine_IOMUX_TX_FUNC                IOMUX_PINCM55_PF_UART3_TX
#define UART_Machine_BAUD_RATE                                          (115200)
#define UART_Machine_IBRD_80_MHZ_115200_BAUD                                (43)
#define UART_Machine_FBRD_80_MHZ_115200_BAUD                                (26)
/* Defines for UART_JY901S */
#define UART_JY901S_INST                                                   UART6
#define UART_JY901S_INST_FREQUENCY                                      80000000
#define UART_JY901S_INST_IRQHandler                             UART6_IRQHandler
#define UART_JY901S_INST_INT_IRQN                                 UART6_INT_IRQn
#define GPIO_UART_JY901S_RX_PORT                                           GPIOB
#define GPIO_UART_JY901S_TX_PORT                                           GPIOB
#define GPIO_UART_JY901S_RX_PIN                                   DL_GPIO_PIN_21
#define GPIO_UART_JY901S_TX_PIN                                   DL_GPIO_PIN_22
#define GPIO_UART_JY901S_IOMUX_RX                                (IOMUX_PINCM49)
#define GPIO_UART_JY901S_IOMUX_TX                                (IOMUX_PINCM50)
#define GPIO_UART_JY901S_IOMUX_RX_FUNC                 IOMUX_PINCM49_PF_UART6_RX
#define GPIO_UART_JY901S_IOMUX_TX_FUNC                 IOMUX_PINCM50_PF_UART6_TX
#define UART_JY901S_BAUD_RATE                                           (115200)
#define UART_JY901S_IBRD_80_MHZ_115200_BAUD                                 (43)
#define UART_JY901S_FBRD_80_MHZ_115200_BAUD                                 (26)
/* Defines for UART_Maixcam */
#define UART_Maixcam_INST                                                  UART5
#define UART_Maixcam_INST_FREQUENCY                                     80000000
#define UART_Maixcam_INST_IRQHandler                            UART5_IRQHandler
#define UART_Maixcam_INST_INT_IRQN                                UART5_INT_IRQn
#define GPIO_UART_Maixcam_RX_PORT                                          GPIOB
#define GPIO_UART_Maixcam_TX_PORT                                          GPIOB
#define GPIO_UART_Maixcam_RX_PIN                                  DL_GPIO_PIN_28
#define GPIO_UART_Maixcam_TX_PIN                                  DL_GPIO_PIN_29
#define GPIO_UART_Maixcam_IOMUX_RX                               (IOMUX_PINCM65)
#define GPIO_UART_Maixcam_IOMUX_TX                               (IOMUX_PINCM66)
#define GPIO_UART_Maixcam_IOMUX_RX_FUNC                IOMUX_PINCM65_PF_UART5_RX
#define GPIO_UART_Maixcam_IOMUX_TX_FUNC                IOMUX_PINCM66_PF_UART5_TX
#define UART_Maixcam_BAUD_RATE                                            (9600)
#define UART_Maixcam_IBRD_80_MHZ_9600_BAUD                                 (520)
#define UART_Maixcam_FBRD_80_MHZ_9600_BAUD                                  (53)
/* Defines for UART_Bluetooth */
#define UART_Bluetooth_INST                                                UART4
#define UART_Bluetooth_INST_FREQUENCY                                   80000000
#define UART_Bluetooth_INST_IRQHandler                          UART4_IRQHandler
#define UART_Bluetooth_INST_INT_IRQN                              UART4_INT_IRQn
#define GPIO_UART_Bluetooth_RX_PORT                                        GPIOB
#define GPIO_UART_Bluetooth_TX_PORT                                        GPIOB
#define GPIO_UART_Bluetooth_RX_PIN                                DL_GPIO_PIN_11
#define GPIO_UART_Bluetooth_TX_PIN                                DL_GPIO_PIN_10
#define GPIO_UART_Bluetooth_IOMUX_RX                             (IOMUX_PINCM28)
#define GPIO_UART_Bluetooth_IOMUX_TX                             (IOMUX_PINCM27)
#define GPIO_UART_Bluetooth_IOMUX_RX_FUNC               IOMUX_PINCM28_PF_UART4_RX
#define GPIO_UART_Bluetooth_IOMUX_TX_FUNC               IOMUX_PINCM27_PF_UART4_TX
#define UART_Bluetooth_BAUD_RATE                                          (9600)
#define UART_Bluetooth_IBRD_80_MHZ_9600_BAUD                               (520)
#define UART_Bluetooth_FBRD_80_MHZ_9600_BAUD                                (53)




/* Defines for SPI_IMU */
#define SPI_IMU_INST                                                       SPI1
#define SPI_IMU_INST_IRQHandler                                 SPI1_IRQHandler
#define SPI_IMU_INST_INT_IRQN                                     SPI1_INT_IRQn
#define GPIO_SPI_IMU_PICO_PORT                                            GPIOB
#define GPIO_SPI_IMU_PICO_PIN                                    DL_GPIO_PIN_15
#define GPIO_SPI_IMU_IOMUX_PICO                                 (IOMUX_PINCM32)
#define GPIO_SPI_IMU_IOMUX_PICO_FUNC                 IOMUX_PINCM32_PF_SPI1_PICO
#define GPIO_SPI_IMU_POCI_PORT                                            GPIOB
#define GPIO_SPI_IMU_POCI_PIN                                    DL_GPIO_PIN_14
#define GPIO_SPI_IMU_IOMUX_POCI                                 (IOMUX_PINCM31)
#define GPIO_SPI_IMU_IOMUX_POCI_FUNC                 IOMUX_PINCM31_PF_SPI1_POCI
/* GPIO configuration for SPI_IMU */
#define GPIO_SPI_IMU_SCLK_PORT                                            GPIOB
#define GPIO_SPI_IMU_SCLK_PIN                                    DL_GPIO_PIN_16
#define GPIO_SPI_IMU_IOMUX_SCLK                                 (IOMUX_PINCM33)
#define GPIO_SPI_IMU_IOMUX_SCLK_FUNC                 IOMUX_PINCM33_PF_SPI1_SCLK



/* Defines for DMA_CH0 */
#define DMA_CH0_CHAN_ID                                                      (0)
#define DMA_CH0_TRIGGER_SEL_FSUB_0                       (DMA_GENERIC_SUB0_TRIG)


/* Port definition for Pin Group IMU */
#define IMU_PORT                                                         (GPIOB)

/* Defines for CS_IMU: GPIOB.12 with pinCMx 29 on package pin 36 */
#define IMU_CS_IMU_PIN                                          (DL_GPIO_PIN_12)
#define IMU_CS_IMU_IOMUX                                         (IOMUX_PINCM29)
/* Port definition for Pin Group BUZZER */
#define BUZZER_PORT                                                      (GPIOB)

/* Defines for Buzzer: GPIOB.1 with pinCMx 13 on package pin 16 */
#define BUZZER_Buzzer_PIN                                        (DL_GPIO_PIN_1)
#define BUZZER_Buzzer_IOMUX                                      (IOMUX_PINCM13)
/* Port definition for Pin Group KEY */
#define KEY_PORT                                                         (GPIOB)

/* Defines for User: GPIOB.31 with pinCMx 68 on package pin 27 */
#define KEY_User_PIN                                            (DL_GPIO_PIN_31)
#define KEY_User_IOMUX                                           (IOMUX_PINCM68)
/* Port definition for Pin Group WAVE */
#define WAVE_PORT                                                        (GPIOC)

/* Defines for Trig: GPIOC.8 with pinCMx 86 on package pin 65 */
#define WAVE_Trig_PIN                                            (DL_GPIO_PIN_8)
#define WAVE_Trig_IOMUX                                          (IOMUX_PINCM86)
/* Port definition for Pin Group LED */
#define LED_PORT                                                         (GPIOA)

/* Defines for L1: GPIOA.14 with pinCMx 36 on package pin 43 */
#define LED_L1_PIN                                              (DL_GPIO_PIN_14)
#define LED_L1_IOMUX                                             (IOMUX_PINCM36)
/* Defines for L2: GPIOA.17 with pinCMx 39 on package pin 54 */
#define LED_L2_PIN                                              (DL_GPIO_PIN_17)
#define LED_L2_IOMUX                                             (IOMUX_PINCM39)
/* Port definition for Pin Group AIN */
#define AIN_PORT                                                         (GPIOB)

/* Defines for AIN1: GPIOB.20 with pinCMx 48 on package pin 67 */
#define AIN_AIN1_PIN                                            (DL_GPIO_PIN_20)
#define AIN_AIN1_IOMUX                                           (IOMUX_PINCM48)
/* Defines for AIN2: GPIOB.24 with pinCMx 52 on package pin 71 */
#define AIN_AIN2_PIN                                            (DL_GPIO_PIN_24)
#define AIN_AIN2_IOMUX                                           (IOMUX_PINCM52)
/* Port definition for Pin Group BIN */
#define BIN_PORT                                                         (GPIOB)

/* Defines for BIN1: GPIOB.25 with pinCMx 56 on package pin 75 */
#define BIN_BIN1_PIN                                            (DL_GPIO_PIN_25)
#define BIN_BIN1_IOMUX                                           (IOMUX_PINCM56)
/* Defines for BIN2: GPIOB.27 with pinCMx 58 on package pin 77 */
#define BIN_BIN2_PIN                                            (DL_GPIO_PIN_27)
#define BIN_BIN2_IOMUX                                           (IOMUX_PINCM58)
/* Port definition for Pin Group GREY */
#define GREY_PORT                                                        (GPIOA)

/* Defines for OUT: GPIOA.27 with pinCMx 60 on package pin 79 */
#define GREY_OUT_PIN                                            (DL_GPIO_PIN_27)
#define GREY_OUT_IOMUX                                           (IOMUX_PINCM60)
/* Defines for AD0: GPIOA.12 with pinCMx 34 on package pin 41 */
#define GREY_AD0_PIN                                            (DL_GPIO_PIN_12)
#define GREY_AD0_IOMUX                                           (IOMUX_PINCM34)
/* Defines for AD1: GPIOA.13 with pinCMx 35 on package pin 42 */
#define GREY_AD1_PIN                                            (DL_GPIO_PIN_13)
#define GREY_AD1_IOMUX                                           (IOMUX_PINCM35)
/* Defines for AD2: GPIOA.15 with pinCMx 37 on package pin 44 */
#define GREY_AD2_PIN                                            (DL_GPIO_PIN_15)
#define GREY_AD2_IOMUX                                           (IOMUX_PINCM37)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_PWM_Motor_init(void);
void SYSCFG_DL_PWM_RGB_init(void);
void SYSCFG_DL_CAPTURE_WAVE_init(void);
void SYSCFG_DL_TIMER_Clock_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_Screen_init(void);
void SYSCFG_DL_UART_Machine_init(void);
void SYSCFG_DL_UART_JY901S_init(void);
void SYSCFG_DL_UART_Maixcam_init(void);
void SYSCFG_DL_UART_Bluetooth_init(void);
void SYSCFG_DL_SPI_IMU_init(void);
void SYSCFG_DL_DMA_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
