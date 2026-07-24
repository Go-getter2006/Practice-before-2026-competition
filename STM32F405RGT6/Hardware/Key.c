#include "main.h"

/*全局变量，用于存储按键键码*/
uint8_t Key_Num;

/**
  * @brief  读取并清除已锁存的按键键码。
  * @retval 1～3：对应按键被按下；0：没有新的按键事件。
  */
uint8_t Key_GetNum(void)
{
	uint8_t Temp;			//定义一个临时变量用于中转
	if (Key_Num)			//如果全局变量的键码不为0
	{
		/*这3句的目的是，实现读取键码并读后清零的效果*/
		Temp = Key_Num;		//先把键码存入临时变量
		Key_Num = 0;		//键码清零
		return Temp;		//返回临时变量，return语句执行后，函数直接结束
	}
	return 0;				//如果if不成立，键码为0，则默认返回0
}

/**
  * @brief  非阻塞读取三个按键的当前状态。
  * @retval 1～3：对应按键当前按下；0：没有按键按下。
  */
uint8_t Key_GetState(void)
{
	if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0)
	{
		return 1;		//直接返回键码1
	}
	if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == 0)
	{
		return 2;		//直接返回键码2
	}
	if (HAL_GPIO_ReadPin(KEY3_GPIO_Port, KEY3_Pin) == 0)
	{
		return 3;		//直接返回键码3
	}
	return 0;			//没有if成立，表示没有按键按下，默认返回0
}

/**
  * @brief  执行按键扫描、消抖和按下沿检测。
  * @note   本函数需要每隔 1 ms 调用一次，内部每 20 ms 扫描一次按键。
  */
void Key_Tick(void)
{
	/*定义静态变量（默认初值为0，函数退出后保留值和存储空间）*/
	static uint8_t Count;					//用于计次分频
	static uint8_t CurrState, PrevState;	//保存按键本次状态和上次状态
	
	Count ++;			//计次自增
	if (Count >= 20)	//如果计次20次，则if成立，即if每隔20ms进一次
	{
		Count = 0;		//计次清零，便于下次计次
		
		/*获取按键的本次状态和上次状态*/
		PrevState = CurrState;			//获取上次状态
		CurrState = Key_GetState();		//获取本次状态
		
		/* 本次状态非0且上次状态为0，表示检测到消抖后的按键按下沿。 */
		if (CurrState != 0 && PrevState == 0)
		{
			/* 保存当前键码；Key_GetNum() 读取后会自动清零。 */
			Key_Num = CurrState;
		}
	}
}
