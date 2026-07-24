#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "stm32f4xx_hal.h"

/* 外部定时器句柄声明 */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/* 硬件引脚配置 */
#define MOTOR_LEFT_IN1_PORT     AIN1_GPIO_Port
#define MOTOR_LEFT_IN1_PIN      AIN1_Pin
#define MOTOR_LEFT_IN2_PORT     AIN2_GPIO_Port
#define MOTOR_LEFT_IN2_PIN      AIN2_Pin
#define MOTOR_LEFT_PWM_TIM      &htim2
#define MOTOR_LEFT_PWM_CHANNEL  TIM_CHANNEL_1
#define MOTOR_LEFT_ENC_TIM      &htim3

#define MOTOR_RIGHT_IN1_PORT    BIN1_GPIO_Port
#define MOTOR_RIGHT_IN1_PIN     BIN1_Pin
#define MOTOR_RIGHT_IN2_PORT    BIN2_GPIO_Port
#define MOTOR_RIGHT_IN2_PIN     BIN2_Pin
#define MOTOR_RIGHT_PWM_TIM     &htim2
#define MOTOR_RIGHT_PWM_CHANNEL TIM_CHANNEL_2
#define MOTOR_RIGHT_ENC_TIM     &htim4

/**
  * @brief 两轮电机初始化
  */
void Motor_Init(void);

/**
  * @brief 设置左电机PWM
  * @param pwm PWM值（-8000 ~ 8000）
  */
void Motor_SetPWM_Left(int16_t pwm);

/**
  * @brief 设置右电机PWM
  * @param pwm PWM值（-8000 ~ 8000）
  */
void Motor_SetPWM_Right(int16_t pwm);

/**
  * @brief 获取左电机编码器速度
  * @return 左电机编码器速度
  */
int16_t Motor_GetSpeed_Left(void);

/**
  * @brief 获取右电机编码器速度
  * @return 右电机编码器速度
  */
int16_t Motor_GetSpeed_Right(void);

#endif /* INC_MOTOR_H_ */
