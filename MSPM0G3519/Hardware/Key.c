#include "Key.h"
#include "ti_msp_dl_config.h"

/*******************************************************************************
 * 每个按键连续采样达到该次数后，确认其状态已经稳定。
 ******************************************************************************/
#define KEY_DEBOUNCE_TICKS \
    (KEY_DEBOUNCE_TIME_MS / KEY_TICK_PERIOD_MS)

#if (KEY_TICK_PERIOD_MS == 0U) || (KEY_DEBOUNCE_TIME_MS < KEY_TICK_PERIOD_MS)
#error "按键扫描周期和消抖时间配置无效"
#endif

typedef struct {
    GPIO_Regs *port;
    uint32_t pin;
} Key_Config_t;

typedef struct {
    uint8_t lastSample;
    uint8_t stableState;
    uint8_t stableCount;
} Key_State_t;

static Key_Config_t g_keyConfig[KEY_NUM_COUNT];
static Key_State_t g_keyState[KEY_NUM_COUNT];
static volatile uint8_t g_keyEvent[KEY_NUM_COUNT];

/*******************************************************************************
 * 名    称： Key_Init
 * 功    能：建立按键硬件映射，初始化软件状态并清除按键事件。
 * 说    明：GPIO 的输入和上拉属性由 SysConfig 配置。
 *           新增按键时，在此处按按键编号补充端口和引脚映射。
 * 参    数：无。
 * 出    口：无返回值。
 ******************************************************************************/
void Key_Init(void)
{
    uint8_t keyNum;
    uint8_t sample;

    g_keyConfig[KEY_NUM_USER].port = KEY_PORT;
    g_keyConfig[KEY_NUM_USER].pin = KEY_User_PIN;

    for (keyNum = KEY_NUM_USER; keyNum < KEY_NUM_COUNT; keyNum++) {
        sample = Key_IsPressed(keyNum);
        g_keyState[keyNum].lastSample = sample;
        g_keyState[keyNum].stableState = sample;
        g_keyState[keyNum].stableCount = 0U;
        g_keyEvent[keyNum] = 0U;
    }
}

/*******************************************************************************
 * 名    称： Key_GetNum
 * 功    能：读取并清除一个按键按下事件。
 * 说    明：每个按键单独保存事件，避免不同按键的事件相互覆盖。
 * 参    数：无。
 * 出    口：按键编号；没有事件时返回 KEY_NUM_NONE。
 ******************************************************************************/
uint8_t Key_GetNum(void)
{
    uint8_t keyNum;
    for (keyNum = KEY_NUM_USER; keyNum < KEY_NUM_COUNT; keyNum++) {
        if (g_keyEvent[keyNum] != 0U) {
            g_keyEvent[keyNum] = 0U;
            return keyNum;
        }
    }

    return KEY_NUM_NONE;
}

/*******************************************************************************
 * 名    称： Key_IsPressed
 * 功    能：读取指定按键的原始电平，并转换为按下状态。
 * 说    明：按键采用内部上拉，低电平表示按下。
 * 参    数：keyNum：需要读取的按键编号。
 * 出    口：1 表示按下，0 表示松开或编号无效。
 ******************************************************************************/
uint8_t Key_IsPressed(uint8_t keyNum)
{
    if ((keyNum == KEY_NUM_NONE) ||
        (keyNum >= KEY_NUM_COUNT) ||
        (g_keyConfig[keyNum].port == 0)) {
        return 0U;
    }

    return (DL_GPIO_readPins(g_keyConfig[keyNum].port,
                             g_keyConfig[keyNum].pin) == 0U) ? 1U : 0U;
}

/*******************************************************************************
 * 名    称： Key_GetUserState
 * 功    能：读取 USER 按键当前的原始按下状态。
 * 参    数：无。
 * 出    口：1 表示按下，0 表示松开。
 ******************************************************************************/
uint8_t Key_GetUserState(void)
{
    return Key_IsPressed(KEY_NUM_USER);
}

/*******************************************************************************
 * 名    称： Key_GetState
 * 功    能：查找当前处于按下状态的按键。
 * 说    明：多个按键同时按下时，优先返回编号较小的按键。
 * 参    数：无。
 * 出    口：按键编号；没有按键按下时返回 KEY_NUM_NONE。
 ******************************************************************************/
uint8_t Key_GetState(void)
{
    uint8_t keyNum;

    for (keyNum = KEY_NUM_USER; keyNum < KEY_NUM_COUNT; keyNum++) {
        if (Key_IsPressed(keyNum) != 0U) {
            return keyNum;
        }
    }

    return KEY_NUM_NONE;
}

/*******************************************************************************
 * 名    称： Key_Tick
 * 功    能：执行一次非阻塞扫描，并在按键稳定按下时记录事件。
 * 说    明：采用连续采样消抖，不调用任何延时函数。
 * 参    数：无。
 * 出    口：无返回值。
 ******************************************************************************/
void Key_Tick(void)
{
    uint8_t keyNum;
    uint8_t sample;
    for (keyNum = KEY_NUM_USER; keyNum < KEY_NUM_COUNT; keyNum++) {
        sample = Key_IsPressed(keyNum);

        if (sample != g_keyState[keyNum].lastSample) {
            g_keyState[keyNum].lastSample = sample;
            g_keyState[keyNum].stableCount = 1U;
            continue;
        }

        if (g_keyState[keyNum].stableCount < KEY_DEBOUNCE_TICKS) {
            g_keyState[keyNum].stableCount++;
        }

        if ((g_keyState[keyNum].stableCount >= KEY_DEBOUNCE_TICKS) &&
            (sample != g_keyState[keyNum].stableState)) {
            if ((g_keyState[keyNum].stableState == 0U) && (sample != 0U)) {
                g_keyEvent[keyNum] = 1U;
            }

            g_keyState[keyNum].stableState = sample;
        }
    }
}
