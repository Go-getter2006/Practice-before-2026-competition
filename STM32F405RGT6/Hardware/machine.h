#ifndef INC_MACHINE_H_
#define INC_MACHINE_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MACHINE_DEFAULT_ID        0x01U
#define MACHINE_DEFAULT_MICROSTEP 32U
#define MACHINE_FRAME_LEN         11U
#define MACHINE_FEEDBACK_FRAME_LEN 9U

typedef enum
{
    MACHINE_DIR_CCW = 0x00U,
    MACHINE_DIR_CW  = 0x01U
} Machine_Direction;

/** TTL 串口反馈数据，对应手册表 3-16。 */
typedef struct
{
    uint8_t id;
    uint8_t target_reached;
    int16_t speed_x10;
    int32_t position_x10;
} Machine_Feedback;

/* USART2 feedback diagnostics for debugger/live-watch inspection. */
extern volatile uint32_t machine_uart2_rx_byte_count;
extern volatile uint32_t machine_feedback_valid_frame_count;
extern volatile uint32_t machine_feedback_reject_count;
extern volatile uint32_t machine_uart2_error_count;

HAL_StatusTypeDef Machine_SetSpeed(uint8_t id, float speed_rad_s, uint8_t microstep);
HAL_StatusTypeDef Machine_SetSpeedEx(uint8_t id,
                                     Machine_Direction dir,
                                     uint8_t microstep,
                                     float speed_rad_s);

HAL_StatusTypeDef Machine_SetTorqueEx(uint8_t id,
                                      Machine_Direction dir,
                                      uint8_t microstep,
                                      float current_ma,
                                      float speed_rad_s);

HAL_StatusTypeDef Machine_SetPosition(uint8_t id,
                                      float angle_deg,
                                      float speed_rad_s,
                                      uint8_t microstep);
HAL_StatusTypeDef Machine_SetPositionEx(uint8_t id,
                                        Machine_Direction dir,
                                        uint8_t microstep,
                                        float angle_deg,
                                        float speed_rad_s);

/**
  * @brief 转到单圈绝对角度，固定使用 32 细分。
  */
HAL_StatusTypeDef Machine_SetAbsolutePosition(uint8_t id,
                                              float angle_deg,
                                              float speed_rad_s);

/**
  * @brief 转到单圈编码器绝对零位。
  */
HAL_StatusTypeDef Machine_ReturnToZero(uint8_t id, float speed_rad_s);

HAL_StatusTypeDef Machine_Enable(uint8_t id);
HAL_StatusTypeDef Machine_Disable(uint8_t id);
HAL_StatusTypeDef Machine_Stop(uint8_t id);
HAL_StatusTypeDef Machine_RequestFeedback(uint8_t id);

/**
  * @brief 启动 USART2反馈的持续中断接收。
  */
HAL_StatusTypeDef Machine_FeedbackRxStart(void);

/**
  * @brief USART2 单字节接收完成回调，由 HAL_UART_RxCpltCallback 调用。
  */
void Machine_FeedbackRxByteCallback(void);

/**
  * @brief USART2 接收错误恢复回调，由 HAL_UART_ErrorCallback 调用。
  */
void Machine_FeedbackRxErrorCallback(void);

/**
  * @brief 读取中断接收缓存中的最近一帧合法反馈。
  * @param feedback 成功时返回最近反馈。
  * @param sequence 可选输出，返回合法反馈递增序号。
  * @retval 1 表示缓存中已有合法反馈；0 表示尚未收到合法反馈。
  */
uint8_t Machine_GetLatestFeedback(Machine_Feedback *feedback,
                                  uint32_t *sequence);

HAL_StatusTypeDef Machine_ReadFeedback(uint8_t id,
                                       Machine_Feedback *feedback,
                                       uint32_t timeout_ms);

uint8_t Machine_BuildFrame(uint8_t frame[MACHINE_FRAME_LEN],
                           uint8_t id,
                           uint8_t mode,
                           Machine_Direction dir,
                           uint8_t microstep,
                           uint16_t position_x10,
                           uint16_t speed_x10);
uint8_t Machine_Checksum(const uint8_t *data, uint8_t len);
uint8_t Machine_IsMicrostepValid(uint8_t microstep);
uint8_t Machine_ParseFeedbackFrame(const uint8_t frame[MACHINE_FEEDBACK_FRAME_LEN],
                                   uint8_t expected_id,
                                   Machine_Feedback *feedback);

#ifdef __cplusplus
}
#endif

#endif /* INC_MACHINE_H_ */
