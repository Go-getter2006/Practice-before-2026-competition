#include "JY901.h"

/* ============ 配置区（请根据实际接线检查） ============ */
extern UART_HandleTypeDef huart5;   // CubeMX生成的UART5句柄

/* ============ 内部变量 ============ */
float yaw = 0.0f;                     // 偏航角（外部可直接读取）
uint8_t uart5_rx_byte = 0;           // 单字节接收缓冲区（serial.c回调中读取）
static uint8_t jy901_buf[11];               // 数据帧暂存
static uint8_t jy901_idx = 0;               // 帧索引

/* ============ 内部函数声明 ============ */
static void JY901_ParseByte(uint8_t data);
static void JY901_SendCmd(const uint8_t *cmd, uint8_t len);

/* ============ 初始化：启动UART5中断接收 ============ */
void JY901_Init(void)
{
    HAL_UART_Receive_IT(&huart5, &uart5_rx_byte, 1);
}

/* ============ 设置角度参考（全轴归零） ============ */
void JY901_SetZeroRef(void)
{
    // 解锁指令：FF AA 69 88 B5
    uint8_t unlock[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
    // 角度参考指令：FF AA 01 08 00（CALSW=0x08）
    uint8_t ref[]    = {0xFF, 0xAA, 0x01, 0x08, 0x00};
    // 保存指令：FF AA 00 00 00
    uint8_t save[]   = {0xFF, 0xAA, 0x00, 0x00, 0x00};
    HAL_Delay(500);
    JY901_SendCmd(unlock, 5);
    HAL_Delay(200);                 // 等待解锁生效
    JY901_SendCmd(ref, 5);
    HAL_Delay(2000);                // 必须等待2秒，传感器稳定计算
    JY901_SendCmd(save, 5);
    HAL_Delay(100);
}

/* ============ 串口接收回调（请在UART5中断回调中调用） ============ */
void JY901_RxCallback(uint8_t data)
{
    JY901_ParseByte(data);
    HAL_UART_Receive_IT(&huart5, &uart5_rx_byte, 1);  // 重新使能接收中断
}

/* ============ 内部：发送指令（阻塞） ============ */
static void JY901_SendCmd(const uint8_t *cmd, uint8_t len)
{
    HAL_UART_Transmit(&huart5, (uint8_t *)cmd, len, 100);
}

/* ============ 内部：逐字节解析协议（提取偏航角） ============ */
static void JY901_ParseByte(uint8_t data)
{
    // 状态机：寻找帧头 0x55，再判断类型为 0x53（角度包）
    if (jy901_idx == 0)
    {
        if (data == 0x55)
        {
            jy901_buf[jy901_idx++] = data;
        }
    }
    else if (jy901_idx == 1)
    {
        if (data == 0x53)          // 只处理角度包
        {
            jy901_buf[jy901_idx++] = data;
        }
        else
        {
            jy901_idx = 0;         // 不是角度包，重新找帧头
        }
    }
    else
    {
        jy901_buf[jy901_idx++] = data;
        if (jy901_idx >= 11)
        {
            jy901_idx = 0;         // 一帧接收完毕

            // 校验和计算（前10字节之和取低8位）
            uint8_t sum = 0;
            for (int i = 0; i < 10; i++)
                sum += jy901_buf[i];

            if (sum == jy901_buf[10])
            {
                // 提取偏航角（字节6为Yaw低8位，字节7为Yaw高8位）
                int16_t yaw_raw = (int16_t)((jy901_buf[7] << 8) | jy901_buf[6]);
                yaw = yaw_raw / 32768.0f * 180.0f;
            }
        }
    }
}
