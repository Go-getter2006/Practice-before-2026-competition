#include "Task.h"
#include "Delay.h"
#include "Greyscale.h"
#include "ICM45686.h"
#include "LED.h"
#include "MahonyAHRS.h"
#include "Maxicam.h"
#include "Motor.h"
#include "Screen.h"
#include "Timer.h"
#include "UART.h"
#include "ti_msp_dl_config.h"

#define TASK1_IMU_PERIOD_MS              (5U)
#define TASK1_RIGHT_PWM_START            (200)
#define TASK1_FIRST_RIGHT_PWM_PEAK       (400)
#define TASK1_FIRST_RIGHT_PWM_END        (200)
#define TASK1_SECOND_RIGHT_PWM_END       (100)
#define TASK1_LEFT_RIGHT_RATIO_X1000     (1212)

volatile uint8_t image = 0U;

static void Task1_ReadImage(void)
{
    Maxicam_Frame_t frame;

    while (Maxicam_ReadFrame(&frame)) {
        if ((frame.length >= 1U) &&
            (frame.data[0] >= 0x01U) && (frame.data[0] <= 0x04U)) {
            image = frame.data[0];
        }
    }
}

static int16_t Task1_RightPwm(float angle, float targetAngle,
                              int16_t peakPwm, int16_t endPwm)
{
    float halfTarget;
    float pwm;

    if (angle < 0.0f) {
        angle = 0.0f;
    } else if (angle > targetAngle) {
        angle = targetAngle;
    }

    halfTarget = targetAngle * 0.5f;
    if (angle <= halfTarget) {
        pwm = (float)TASK1_RIGHT_PWM_START +
              ((float)(peakPwm - TASK1_RIGHT_PWM_START) *
               angle / halfTarget);
    } else {
        pwm = (float)peakPwm +
              ((float)(endPwm - peakPwm) *
               (angle - halfTarget) / (targetAngle - halfTarget));
    }

    return (int16_t)(pwm + 0.5f);
}

static void Task1_DriveCircle(float angle, float targetAngle,
                              int16_t peakPwm, int16_t endPwm)
{
    int16_t rightPwm;
    int16_t leftPwm;
    int16_t trackError;

    rightPwm = Task1_RightPwm(angle, targetAngle, peakPwm, endPwm);
    leftPwm = (int16_t)(((int32_t)rightPwm *
                         TASK1_LEFT_RIGHT_RATIO_X1000 + 500) / 1000);

    trackError = (int16_t)Track_err(Get_Grayscale_State());
    leftPwm -= trackError;
    rightPwm += trackError;

    /* 灰度纠偏作用于 PWM 幅值；左右电机镜像安装，所以右轮命令取负。 */
    Motor_SetSpeed(leftPwm, -rightPwm);
}

static void Task1_RunToAngle(float targetAngle, int16_t peakPwm,
                             int16_t endPwm,
                             float accel_mg[3], float gyro_dps[3])
{
    float ypr[3];
    float previousYaw;
    float deltaYaw;
    float angle = 0.0f;
    uint32_t lastUpdate = nowtime;

    MahonyAHRS_GetYawPitchRoll(ypr);
    previousYaw = ypr[0];
    Task1_DriveCircle(0.0f, targetAngle, peakPwm, endPwm);

    while (angle < targetAngle) {
        Task1_ReadImage();

        if ((uint32_t)(nowtime - lastUpdate) < TASK1_IMU_PERIOD_MS) {
            continue;
        }
        lastUpdate = nowtime;

        ICM45686_ReadAccelGyro(accel_mg, gyro_dps);
        MahonyAHRS_Update(gyro_dps[0], gyro_dps[1], gyro_dps[2],
                          accel_mg[0], accel_mg[1], accel_mg[2]);
        MahonyAHRS_GetYawPitchRoll(ypr);

        /* 展开 +/-180 度跳变，得到本次出发后连续的顺时针转角。 */
        deltaYaw = ypr[0] - previousYaw;
        if (deltaYaw < -180.0f) {
            deltaYaw += 360.0f;
        } else if (deltaYaw > 180.0f) {
            deltaYaw -= 360.0f;
        }
        previousYaw = ypr[0];

        angle += deltaYaw;
        if (angle < 0.0f) {
            angle = 0.0f;
        }
        Task1_DriveCircle(angle, targetAngle, peakPwm, endPwm);
    }

    Motor_Stop();
}

/*******************************************************************************
 * 名    称：task1
 * 功    能：执行任务一，通过调试串口发送任务执行提示。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void task1(void)
{
    static const uint8_t bluetoothStopFrame[3] = {0xFFU, 0xAAU, 0xFEU};
    float accel_mg[3];
    float gyro_dps[3];
    uint8_t capturedImage;
    int16_t secondPeakPwm;
    float targetAngle;

    image = 0U;

    /* 第一圈：0~180 度加速，180~360 度减速，回到 0 度停车。 */
    Task1_RunToAngle(360.0f, TASK1_FIRST_RIGHT_PWM_PEAK,
                     TASK1_FIRST_RIGHT_PWM_END, accel_mg, gyro_dps);
    Task1_ReadImage();
    capturedImage = image;

    switch (capturedImage) {
        case 0x01U:
            targetAngle = 90.0f;
            secondPeakPwm = 300;
            LED_RGB_Set(0U, 15U, 0U);
            break;

        case 0x02U:
            targetAngle = 180.0f;
            secondPeakPwm = 400;
            LED_RGB_Set(15U, 0U, 0U);
            break;

        case 0x03U:
            targetAngle = 270.0f;
            secondPeakPwm = 400;
            LED_RGB_Set(0U, 0U, 15U);
            break;

        case 0x04U:
            targetAngle = 360.0f;
            secondPeakPwm = 400;
            LED_RGB_Set(15U, 15U, 15U);
            break;

        default: return;
    }

    image = 0U;
    Task1_RunToAngle(targetAngle, secondPeakPwm,
                     TASK1_SECOND_RIGHT_PWM_END, accel_mg, gyro_dps);

    LED_RGB_Set(0U, 0U, 0U);
    UART_Bluetooth_SendData(bluetoothStopFrame,
                            (uint32_t)sizeof(bluetoothStopFrame));
    DL_GPIO_setPins(BUZZER_PORT, BUZZER_Buzzer_PIN);
    Delay_ms(1000U);
    DL_GPIO_clearPins(BUZZER_PORT, BUZZER_Buzzer_PIN);
}

/*******************************************************************************
 * 名    称：task2
 * 功    能：执行任务二，点亮 L1 指示灯。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void task2(void)
{
    LED_L1_On();
}

/*******************************************************************************
 * 名    称：task3
 * 功    能：执行任务三，熄灭 L1 指示灯。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void task3(void)
{
    LED_L1_Off();
}

/*******************************************************************************
 * 名    称：Task_Process
 * 功    能：根据串口屏任务号调用对应任务函数。
 * 参    数：无。
 * 出    口：无返回值。
 * 说    明：未定义的任务号不会执行任何操作。
 *******************************************************************************/
void Task_Process(void)
{
    uint8_t taskNumber;

    while (Screen_ReadTask(&taskNumber)) {
        switch (taskNumber) {
            case SCREEN_TASK_1:
                task1();
                break;

            case SCREEN_TASK_2:
                task2();
                break;

            case SCREEN_TASK_3:
                task3();
                break;

            default:
                break;
        }
    }
}
