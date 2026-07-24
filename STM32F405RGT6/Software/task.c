#include "task.h"
#include "main.h"
#include "OLED.h"
#include "serial.h"
#include "motion.h"
#include "motor.h"
#include "Delay.h"
#include "JY901.h"
#include "PID.h"
#include <math.h>
#include "string.h"

extern volatile int32_t task1_encoder_acc;
extern volatile int32_t task1_encoder_right_acc;
extern volatile uint8_t task1_record_encoder;
extern volatile int32_t task3_encoder_acc;
extern volatile uint8_t task3_record_encoder;
extern float left_target, right_target;
extern uint16_t all_state;
extern uint8_t angle_pid_enable;
extern float pos_encoder_acc;
extern float task2_start_speed;
extern float task2_speed_step;
extern uint16_t task2_poskp_ms;
extern PID_t PosPID;
extern PID_t AnglePID;
extern int32_t angle_balance_target;

#define DISTANCE_1M     270000
#define HALF_DISTANCE   (DISTANCE_1M / 2)
#define MAX_TARGET      5000

/**
  * @brief  执行直行约 1 m 的梯形速度状态机。
  * @note   根据累计编码器里程依次完成加速、匀速、减速、停车和结束提示。
  */
void Task1(void)
{
  static uint8_t  task_state = 0;
  static uint8_t  dir        = 0;
  static float    speed      = 0;

  switch (task_state)
  {
    case 0:
    {
      task1_encoder_acc = 0;
      task1_encoder_right_acc = 0;
      task1_record_encoder = 1;
      left_target = 0;
      right_target = 0;
      speed = 0;
      dir = 0;

      task_state = 1;
      break;
    }

    case 1:
    {
      int32_t avg = (task1_encoder_acc + task1_encoder_right_acc) / 2;

      if (avg >= HALF_DISTANCE && dir == 0) {
        dir = 1;
      }

      if (dir == 0) {
        speed += 100;
        if (speed > MAX_TARGET) speed = MAX_TARGET;
      } else {
        speed -= 100;
        if (speed < 0) speed = 0;
      }

      left_target = speed;
      right_target = speed;

      OLED_Printf(0, 32, OLED_8X16, "%.1fcm", avg / 2742.4f);

      if (avg >= DISTANCE_1M - 10000) {
        task1_record_encoder = 0;
        left_target = 0;
        right_target = 0;
        task_state = 99;
      }
      break;
    }

    case 99:
    {
      static uint8_t  end_seq  = 0;
      static uint32_t end_tick = 0;
      left_target = 0;
      right_target = 0;
      if (end_seq == 0) 
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        end_tick = HAL_GetTick();
        end_seq = 1;
      }
      else if (end_seq == 1 && HAL_GetTick() - end_tick >= 1000) 
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        end_seq = 2;
        all_state = 0;
        task_state = 0;
        end_seq = 0;
      }
      break;
    }

    default:
      task_state = 0;
      break;
  }
}

/**
  * @brief  执行任务 2 的减速、等待和平衡参数切换状态机。
  * @note   本函数只维护任务状态和 PID 参数，电机控制由周期控制任务执行。
  */
void Task2(void)
{
  static uint8_t  task_state = 0;
  static uint32_t start_tick = 0;
  static uint32_t angle_start_tick = 0;
  static float    speed = 0;
  static float    saved_angle_ki = 0;
  static float    saved_angle_kd = 0;

  switch (task_state)
  {
    case 0:
    {
      speed = task2_start_speed;
      left_target = speed;
      right_target = speed;
      task_state = 1;
      break;
    }

    case 1:
    {
      speed -= task2_speed_step;
      if (speed < 0) speed = 0;
      left_target = speed;
      right_target = speed;

      if (speed <= 0)
      {
        left_target = 0;
        right_target = 0;
        angle_pid_enable = 1;
        pos_encoder_acc = 0;
        PosPID.kp = 0.0015f;
        saved_angle_ki = AnglePID.ki;
        saved_angle_kd = AnglePID.kd;
        AnglePID.ki = 0;
        AnglePID.kd = saved_angle_kd * 0.5f;
        start_tick = HAL_GetTick();
        task_state = 2;
      }
      break;
    }

    case 2:
    {
      uint32_t elapsed = HAL_GetTick() - start_tick;

      if (elapsed >= task2_poskp_ms && PosPID.kp != 0)
      {
        PosPID.kp = 0;
      }

      if (elapsed >= 500)
      {
        AnglePID.ki = saved_angle_ki;
        AnglePID.kd = saved_angle_kd;
        angle_start_tick = HAL_GetTick();
        task_state = 3;
      }
      break;
    }

    case 3:
    {
      if (HAL_GetTick() - angle_start_tick >= 12000)
      {
        angle_pid_enable = 0;
        left_target = 0;
        right_target = 0;
        task_state = 99;
      }
      break;
    }

    case 99:
    {
      static uint8_t  end_seq  = 0;
      static uint32_t end_tick = 0;
      left_target = 0;
      right_target = 0;
      if (end_seq == 0)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        end_tick = HAL_GetTick();
        end_seq = 1;
      }
      else if (end_seq == 1 && HAL_GetTick() - end_tick >= 1000)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        end_seq = 2;
        all_state = 0;
        task_state = 0;
        end_seq = 0;
      }
      break;
    }

    default:
      task_state = 0;
      break;
  }
}

/**
  * @brief  执行任务 3 的平衡、往复目标切换和短距离行驶状态机。
  * @note   往复阶段每 100 ms 切换一次目标角度，完成后按编码器里程停车。
  */
void Task3(void)
{
  static uint8_t  task_state = 0;
  static uint32_t tick = 0;
  static int8_t   osc_index = 0;
  static float    saved_angle_ki = 0;
  static float    saved_angle_kd = 0;

  switch (task_state)
  {
    case 0:
    {
      task3_encoder_acc = 0;
      task3_record_encoder = 1;
      angle_pid_enable = 0;
      angle_balance_target = 1430;
      left_target = 2000;
      right_target = 2000;
      tick = HAL_GetTick();
      task_state = 1;
      break;
    }

    case 1:
    {
      if (HAL_GetTick() - tick >= 100)
      {
        tick = HAL_GetTick();
        osc_index = 0;
        task_state = 2;
      }
      break;
    }

    case 2:
    {
      if (HAL_GetTick() - tick >= 100)
      {
        osc_index++;
        tick = HAL_GetTick();

        if (osc_index >= 8)
        {
          left_target = 0;
          right_target = 0;
          angle_pid_enable = 1;
          pos_encoder_acc = 0;
          PosPID.kp = 0.0015f;
          saved_angle_ki = AnglePID.ki;
          saved_angle_kd = AnglePID.kd;
          AnglePID.ki = 0;
          AnglePID.kd = saved_angle_kd * 0.5f;
          tick = HAL_GetTick();
          task_state = 3;
        }
        else
        {
          switch (osc_index)
          {
            case 0: left_target = 2000;  right_target = 2000;  break;
            case 1: left_target = -2000; right_target = -2000; break;
            case 2: left_target = 1500;  right_target = 1500;  break;
            case 3: left_target = -1500; right_target = -1500; break;
            case 4: left_target = 1000;  right_target = 1000;  break;
            case 5: left_target = -1000; right_target = -1000; break;
            case 6: left_target = 500;   right_target = 500;   break;
            case 7: left_target = -500;  right_target = -500;  break;
          }
        }
      }
      break;
    }

    case 3:
    {
      uint32_t elapsed = HAL_GetTick() - tick;

      if (elapsed >= task2_poskp_ms && PosPID.kp != 0)
      {
        PosPID.kp = 0;
      }

      if (elapsed >= 500)
      {
        AnglePID.ki = saved_angle_ki;
        AnglePID.kd = saved_angle_kd;
        task_state = 4;
      }
      break;
    }

    case 4:
    {
      if (task3_encoder_acc >= 27000)
      {
        angle_pid_enable = 0;
        left_target = 0;
        right_target = 0;
        task3_record_encoder = 0;
        task_state = 99;
      }
      break;
    }

    case 99:
    {
      static uint8_t  end_seq  = 0;
      static uint32_t end_tick = 0;
      left_target = 0;
      right_target = 0;
      if (end_seq == 0)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        end_tick = HAL_GetTick();
        end_seq = 1;
      }
      else if (end_seq == 1 && HAL_GetTick() - end_tick >= 1000)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        end_seq = 2;
        angle_balance_target = 1436;
        all_state = 0;
        task_state = 0;
        osc_index = 0;
        end_seq = 0;
      }
      break;
    }

    default:
      task_state = 0;
      osc_index = 0;
      break;
  }
}

/**
  * @brief  在 OLED 上显示任务 4 提示信息。
  */
void Task4(void)
{
  OLED_Printf(0, 0, OLED_8X16, "Task 4");
  OLED_Update();
}

/**
  * @brief  在 OLED 上显示任务 5 提示信息。
  */
void Task5(void)
{
  OLED_Printf(0, 0, OLED_8X16, "Task 5");
  OLED_Update();
}
