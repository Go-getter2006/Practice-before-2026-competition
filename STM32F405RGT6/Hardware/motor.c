#include "motor.h"
#include "main.h"
#include "stm32f4xx_hal_gpio.h"
#include "tim.h"

/**
  * @brief  初始化左右编码器、电机 PWM 和驱动器使能状态。
  */
void Motor_Init(void)
{
    HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);
    /* 启动编码器定时器 */
    HAL_TIM_Encoder_Start(MOTOR_LEFT_ENC_TIM, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(MOTOR_RIGHT_ENC_TIM, TIM_CHANNEL_ALL);

    /* 启动PWM定时器 */
    HAL_TIM_PWM_Start(MOTOR_LEFT_PWM_TIM, MOTOR_LEFT_PWM_CHANNEL);
    HAL_TIM_PWM_Start(MOTOR_RIGHT_PWM_TIM, MOTOR_RIGHT_PWM_CHANNEL);

    /* 初始PWM设为0 */
    __HAL_TIM_SET_COMPARE(MOTOR_LEFT_PWM_TIM, MOTOR_LEFT_PWM_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(MOTOR_RIGHT_PWM_TIM, MOTOR_RIGHT_PWM_CHANNEL, 0);
}

/**
  * @brief  设置左电机的转向和 PWM 比较值。
  * @param  pwm 有符号 PWM 指令，范围为 -8000～8000，超出范围时自动限幅。
  */
void Motor_SetPWM_Left(int16_t pwm)   // 修改为 int16_t
{
    uint16_t pwm_val;

    /* 边界限制（ARR=8400，按8000标度） */
    if(pwm > 8000) pwm = 8000;
    else if(pwm < -8000) pwm = -8000;

    /* 设置方向并输出PWM */
    if(pwm >= 0)
    {
        /* 正转 */
        HAL_GPIO_WritePin(MOTOR_LEFT_IN1_PORT, MOTOR_LEFT_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_LEFT_IN2_PORT, MOTOR_LEFT_IN2_PIN, GPIO_PIN_SET);
        pwm_val = (uint16_t)pwm;
    }
    else
    {
        /* 反转 */
        HAL_GPIO_WritePin(MOTOR_LEFT_IN1_PORT, MOTOR_LEFT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_LEFT_IN2_PORT, MOTOR_LEFT_IN2_PIN, GPIO_PIN_RESET);
        pwm_val = (uint16_t)(-pwm);
    }

    /* 设置PWM比较值 */
    __HAL_TIM_SET_COMPARE(MOTOR_LEFT_PWM_TIM, MOTOR_LEFT_PWM_CHANNEL, pwm_val);
}

/**
  * @brief  设置右电机的转向和 PWM 比较值。
  * @param  pwm 有符号 PWM 指令，范围为 -8000～8000，超出范围时自动限幅。
  */
void Motor_SetPWM_Right(int16_t pwm)   // 修改为 int16_t
{
    uint16_t pwm_val;

    /* 边界限制（ARR=8400，按8000标度） */
    if(pwm > 8000) pwm = 8000;
    else if(pwm < -8000) pwm = -8000;

    /* 设置方向并输出PWM */
    if(pwm >= 0)
    {
        /* 正转 */
        HAL_GPIO_WritePin(MOTOR_RIGHT_IN1_PORT, MOTOR_RIGHT_IN1_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(MOTOR_RIGHT_IN2_PORT, MOTOR_RIGHT_IN2_PIN, GPIO_PIN_RESET);
        pwm_val = (uint16_t)pwm;
    }
    else
    {
        /* 反转 */
        HAL_GPIO_WritePin(MOTOR_RIGHT_IN1_PORT, MOTOR_RIGHT_IN1_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(MOTOR_RIGHT_IN2_PORT, MOTOR_RIGHT_IN2_PIN, GPIO_PIN_SET);
        pwm_val = (uint16_t)(-pwm);
    }

    /* 设置PWM比较值 */
    __HAL_TIM_SET_COMPARE(MOTOR_RIGHT_PWM_TIM, MOTOR_RIGHT_PWM_CHANNEL, pwm_val);
}

/**
  * @brief  读取并清零左编码器计数，同时修正左轮方向符号。
  * @retval 自上次读取以来的左编码器增量，前进方向为正。
  */
int16_t Motor_GetSpeed_Left(void)
{
    int16_t speed;
    speed = __HAL_TIM_GET_COUNTER(MOTOR_LEFT_ENC_TIM);
    __HAL_TIM_SET_COUNTER(MOTOR_LEFT_ENC_TIM, 0);
    return -speed;  /* 左右电机速度相反，才能正常行驶 */
}

/**
  * @brief  读取并清零右编码器计数。
  * @retval 自上次读取以来的右编码器增量，前进方向为正。
  */
int16_t Motor_GetSpeed_Right(void)
{
    int16_t speed;
    speed = __HAL_TIM_GET_COUNTER(MOTOR_RIGHT_ENC_TIM);
    __HAL_TIM_SET_COUNTER(MOTOR_RIGHT_ENC_TIM, 0);
    return speed;  /* 左右电机速度相反，才能正常行驶 */
}
