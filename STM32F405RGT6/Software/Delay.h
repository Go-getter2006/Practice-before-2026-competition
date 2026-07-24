#ifndef INC_DELAY_H_
#define INC_DELAY_H_


#include <stdint.h>

/**
  * @brief 延时初始化函数（在 main 中调用一次）
  * @note  计算微秒延时所需的时钟系数
  */
void Delay_Init(void);

/**
  * @brief 微秒级延时
  * @param nus 延时的微秒数，范围：0~0xFFFFFFFF（受实际计数限制，最长约1.2小时@72MHz）
  */
void Delay_us(uint32_t nus);

/**
  * @brief 毫秒级延时（基于微秒实现）
  * @param nms 延时的毫秒数
  */
void Delay_ms(uint32_t nms);

/**
  * @brief 秒级延时（基于毫秒实现）
  * @param ns 延时的秒数
  */
void Delay_s(uint32_t ns);

#endif /* INC_DELAY_H_ */
