#include "motion.h"
#include "motor.h"
#include "main.h"

float error = 0.0f;                     // 全局巡线误差

// 选择传感器通道
static void _select_channel(uint8_t channel)
{
  HAL_GPIO_WritePin(AD0_GPIO_Port, AD0_Pin, (channel >> 0) & 0x01); // bit0 -> AD0
  HAL_GPIO_WritePin(AD1_GPIO_Port, AD1_Pin, (channel >> 1) & 0x01); // bit1 -> AD1
  HAL_GPIO_WritePin(AD2_GPIO_Port, AD2_Pin, (channel >> 2) & 0x01); // bit2 -> AD2
}

// 读取OUT引脚的值
static uint16_t Read_OUT_value(void)
{
  return HAL_GPIO_ReadPin(OUT_GPIO_Port, OUT_Pin);
}

// 初始化灰度传感器所需的GPIO
void Grayscale_Sensor_Init(void)
{
  // GPIO已经在CubeMX中配置，这里不需要重新初始化
  // 只需要确保引脚状态正确
  _select_channel(0);
}

// 读取所有8个通道的灰度值
void Grayscale_Sensor_Read_All(uint16_t* sensor_values)
{
  uint8_t i;
  for (i = 0; i < GRAYSCALE_SENSOR_CHANNELS; i++)
  {
    _select_channel(i);
    Delay_us(100);
    sensor_values[i] = Read_OUT_value();
  }
}

// 读取单个指定通道的灰度值
uint16_t Grayscale_Sensor_Read_Single(uint8_t channel)
{
  if (channel >= GRAYSCALE_SENSOR_CHANNELS)
  {
    return 0; // 无效通道
  }
  _select_channel(channel);
  Delay_us(50);
  return Read_OUT_value();
}

// 获取灰度传感器状态
uint8_t Get_Grayscale_State(void)
{
  uint16_t sensor_values[8];
  Grayscale_Sensor_Read_All(sensor_values);
  
  uint8_t state = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (sensor_values[i]) {
      state |= (1 << (7-i));
    }
  }
  return state;
}

// 巡线误差计算
float Track_err(void)
{
  static uint8_t last_sensor_state = 0;
  
  uint8_t state = sensor_state; // 使用主循环中已经读取的传感器状态
  
  if (state != last_sensor_state)
  {
    last_sensor_state = state;
    switch(state)
    {
      case 0:     //0000 0000   
      error= 0 ;     break;
      case 16:    //0001 0000   
      error= 1 ;  break;
      case 8:     //0000 1000   
      error= -1 ; break;    
      case 24:    //0001 1000   
      error= 0 ;     break;    
      case 60:    //0011 1100   
      error= 0 ;     break;    
      case 126:   //0111 1110   
      error= 0 ;     break;
      
      case 48:     //0011 0000   //小车右偏，err为正
      error= 1;     break;
      case 32:     //0010 0000   
      error= 2 ;     break;
      case 64:     //0100 0000   
      error= 3 ;     break;
      case 96:     //0110 0000   
      error= 3 ;     break;
      case 128:    //1000 0000   
      error= 4 ;     break;    
      case 192:    //1100 0000   
      error= 4 ;     break;
      // case 224:   //1110 0000   
      // error= 9 ;     break;
      // case 160:   //1010 0000   
      // error= 9;     break;
      // case 252:   //1111 1100   
      // error= 9 ;     break;
      // case 248:   //1111 1000   
      // error= 9 ;     break;
      // case 240:   //1111 0000   
      // error= 9 ;     break;
      // case 124:   //0111 1100   
      // error= 9 ;     break;
      // case 120:   //0111 1000   
      // error= 9 ;     break;
      case 56:    //0011 1000   
      error= 1 ;     break;
      case 12:   //0000 1100   //小车左偏，err为负
      error= -1;     break;    
      // case 14:   //0000 1110   //小车左偏，err为负
      // error= -6;     break;    
      // case 30:   //0001 1110   //小车左偏，err为负
      // error= -9 ;     break;
      // case 62:   //0011 1110   //小车左偏，err为负
      // error= -9 ;     break;
      // case 31:   //0001 1111   
      // error= -9 ;     break;
      // case 63:   //0011 1111   
      // error= -9 ;     break;
      case 4:    //0000 0100   
      error= -1 ;     break;     
      case 2:    //0000 0010   
      error= -3 ;     break;
      case 6:    //0000 0110   
      error= -3 ;     break;
      case 1:    //0000 0001   
      error= -4 ;     break;
      case 3:    //0000 0011   
      error= -4 ;     break;
      // case 7:    //0000 0111   
      // error= -12 ;     break;
      // case 15:   //0000 1111   
      // error= -12 ;     break;
      
      default:
      // 优化点1：默认状态不重置error为0，沿用当前值，避免无效跳变
      break;
    }
  }
  
  return error;
}

