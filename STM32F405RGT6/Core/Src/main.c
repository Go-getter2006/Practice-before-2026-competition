/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : MS42DDC feedback and lidar display test
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "gpio.h"
#include "usart.h"

/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "Lidar.h"
#include "machine.h"
#include "Bluetooth.h"
#include "serial.h"
#include "PID.h"
#include <stdint.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
#define DISPLAY_PERIOD_MS       50U
#define MOTOR_POWER_ON_WAIT_MS  1000U
#define MOTOR_ZERO_WAIT_MS      2000U
#define MOTOR_TX_TIMEOUT_MS      100U
#define MOTOR_MOVE_CURRENT_MA   1000.0f
#define MOTOR_MOVE_SPEED_RAD_S     8.0f
#define MOTOR_NEAR_ANGLE_DEG       20.0f
#define MOTOR_MODE_SWITCH_WAIT_MS  50U
/* USER CODE END PD */

/*
 * These compatibility globals are retained because task.c is still part of
 * the Keil project.  The minimal diagnostic main loop does not use them.
 */
/* USER CODE BEGIN PV */
uint8_t sensor_state = 0U;
float left_target = 0.0f;
float right_target = 0.0f;
uint16_t all_state = 0U;
uint8_t angle_pid_enable = 0U;
float pos_encoder_acc = 0.0f;
float task2_start_speed = 3000.0f;
float task2_speed_step = 20.0f;
uint16_t task2_poskp_ms = 500U;
int32_t angle_balance_target = 1436;

volatile int32_t task1_encoder_acc = 0;
volatile int32_t task1_encoder_right_acc = 0;
volatile uint8_t task1_record_encoder = 0U;
volatile int32_t task3_encoder_acc = 0;
volatile uint8_t task3_record_encoder = 0U;

PID_t PosPID = {0};
PID_t AnglePID = {0};

static float motor_angle_deg = 0.0f;
static uint8_t motor_angle_valid = 0U;
static uint32_t motor_feedback_sequence = 0U;

static float lidar_angle_deg = 0.0f;
static uint8_t lidar_data_valid = 0U;

/* 0x04 single-turn absolute mode: 5 rad/s to 0 degrees. */
static uint8_t motor_zero_command[MACHINE_FRAME_LEN] = {
  0x7BU, 0x01U, 0x04U, 0x01U, 0x20U, 0x00U,
  0x00U, 0x00U, 0x32U, 0x6DU, 0x7DU
};

/* 0x03 torque mode: 1 mA and 0 rad/s. */
static uint8_t motor_probe_current_command[MACHINE_FRAME_LEN] = {
  0x7BU, 0x01U, 0x03U, 0x01U, 0x20U, 0x00U,
  0x01U, 0x00U, 0x00U, 0x59U, 0x7DU
};

/* 0x03 torque mode: restore 1000 mA and keep speed at 0 rad/s. */
static uint8_t motor_work_current_command[MACHINE_FRAME_LEN] = {
  0x7BU, 0x01U, 0x03U, 0x01U, 0x20U, 0x03U,
  0xE8U, 0x00U, 0x00U, 0xB3U, 0x7DU
};
/* USER CODE END PV */

void SystemClock_Config(void);

/* USER CODE BEGIN 0 */
/**
  * @brief Copy the newest BCC-checked MS42DDC feedback frame.
  * @note Feedback layout:
  *       [0] address, [1] flag, [2..3] speed x10,
  *       [4..7] signed position x10, [8] BCC.
  */
static void App_UpdateMotorFeedback(void)
{
  Machine_Feedback feedback;
  uint32_t sequence;
  int32_t single_turn_position_x10;

  if (Machine_GetLatestFeedback(&feedback, &sequence) &&
      (sequence != motor_feedback_sequence))
  {
    motor_feedback_sequence = sequence;

    /* Convert signed accumulated position to the single-turn range 0..359.9. */
    single_turn_position_x10 = feedback.position_x10 % 3600L;
    if (single_turn_position_x10 < 0L)
    {
      single_turn_position_x10 += 3600L;
    }

    /* Example: 0x00000E0F = 3599, so the angle is 359.9 degrees. */
    motor_angle_deg = (float)single_turn_position_x10 / 10.0f;
    motor_angle_valid = 1U;
  }
}

/**
  * @brief Preserve the most recent valid lidar target for stable display.
  */
static void App_UpdateLidarData(void)
{
  if (car_angle_valid)
  {
    lidar_angle_deg = car_angle;
    lidar_data_valid = 1U;
  }
}

/**
  * @brief Display only the decoded motor angle and lidar result.
  */
static void App_UpdateOled(void)
{
  OLED_Clear();

  if (motor_angle_valid)
  {
    OLED_Printf(0, 0, OLED_8X16, "Motor:%7.1f", motor_angle_deg);
  }
  else
  {
    OLED_Printf(0, 0, OLED_8X16, "Motor:  --.-");
  }

  if (lidar_data_valid)
  {
    OLED_Printf(0, 16, OLED_8X16, "Radar:%7.1f", lidar_angle_deg);
  }
  else
  {
    OLED_Printf(0, 16, OLED_8X16, "Radar:  --.-");
  }

  OLED_Update();
}

/** Calculate the requested reverse relative movement. */
static float App_CalculateMoveAngle(float motor_deg,
                                    float radar_deg)
{
  float difference = radar_deg - motor_deg;
  float absolute_difference = difference;

  if (absolute_difference < 0.0f)
  {
    absolute_difference = -absolute_difference;
  }

  if (absolute_difference <= MOTOR_NEAR_ANGLE_DEG)
  {
    return 360.0f;
  }

  if (difference > 0.0f)
  {
    return difference;
  }

  if (difference < 0.0f)
  {
    return 360.0f + difference;
  }

  return 0.0f;
}

/** Start the first movement when USART6 receives FF AA FE. */
static void App_HandleUsart6Command(void)
{
  Bluetooth_Command command = Bluetooth_GetCommand();
  float move_angle_deg;

  if ((command != BLUETOOTH_COMMAND_ALIGN_LIDAR) ||
      !motor_angle_valid || !lidar_data_valid)
  {
    return;
  }

  move_angle_deg = App_CalculateMoveAngle(motor_angle_deg,
                                         lidar_angle_deg);

  /* Restore working current, then start the first reverse relative movement. */
  if (Machine_SetTorqueEx(MACHINE_DEFAULT_ID,
                          MACHINE_DIR_CW,
                          MACHINE_DEFAULT_MICROSTEP,
                          MOTOR_MOVE_CURRENT_MA,
                          0.0f) != HAL_OK)
  {
    return;
  }

  HAL_Delay(MOTOR_MODE_SWITCH_WAIT_MS);
  (void)Machine_SetPositionEx(MACHINE_DEFAULT_ID,
                              MACHINE_DIR_CCW,
                              MACHINE_DEFAULT_MICROSTEP,
                              move_angle_deg,
                              MOTOR_MOVE_SPEED_RAD_S);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  uint32_t display_tick;

  HAL_Init();
  SystemClock_Config();

  /* Only the peripherals required by this diagnostic are initialized. */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();

  OLED_Init();
  App_UpdateOled();

  /* Arm both receive paths immediately, before the motor startup delays. */
  Bluetooth_Init();
  USART6_RX_Init();
  Lidar_Init();
  while (Machine_FeedbackRxStart() != HAL_OK)
  {
    HAL_Delay(1U);
  }

  /* Power-on sequence: wait 1 s and restore enough current for homing. */
  HAL_Delay(MOTOR_POWER_ON_WAIT_MS);
  if (HAL_UART_Transmit(&huart2,
                        motor_work_current_command,
                        MACHINE_FRAME_LEN,
                        MOTOR_TX_TIMEOUT_MS) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_Delay(MOTOR_MODE_SWITCH_WAIT_MS);

  /* Go to the single-turn absolute zero position at 5 rad/s. */
  if (HAL_UART_Transmit(&huart2,
                        motor_zero_command,
                        MACHINE_FRAME_LEN,
                        MOTOR_TX_TIMEOUT_MS) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_Delay(MOTOR_ZERO_WAIT_MS);
  if (HAL_UART_Transmit(&huart2,
                        motor_probe_current_command,
                        MACHINE_FRAME_LEN,
                        MOTOR_TX_TIMEOUT_MS) != HAL_OK)
  {
    Error_Handler();
  }

  display_tick = HAL_GetTick();

  while (1)
  {
    /* Keep the existing lidar scan parsing and car-angle calculation. */
    Lidar_Process();
    App_UpdateLidarData();

    /* Consume motor push frames; the driver uses a 9-byte sliding window. */
    App_UpdateMotorFeedback();
    App_HandleUsart6Command();

    if ((HAL_GetTick() - display_tick) >= DISPLAY_PERIOD_MS)
    {
      display_tick = HAL_GetTick();
      App_UpdateOled();
    }
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif
