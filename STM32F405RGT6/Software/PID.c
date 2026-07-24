#include "stm32f4xx.h"                  // Device header
#include "main.h"
#include "PID.h"

/**
  * @brief  清零 PID 运行状态，保留已配置的参数和输出限幅。
  * @param  p PID 控制器结构体指针。
  */
void PID_Init(PID_t *p)
{
	/*把PID表示状态的参数清零，避免之前遗留的参数对本次启动造成影响*/
	p->target = 0;
	p->actual = 0;
	p->out = 0;
	p->error0 = 0;
	p->error1 = 0;
	p->errorInt = 0;
}

/**
  * @brief  根据结构体中的目标值和实际值执行一次位置式 PID 计算。
  * @param  p PID 控制器结构体指针，计算结果写回 p->out。
  * @note   本函数会对误差积分和最终输出进行限幅。
  */
void PID_Update(PID_t *p)
{
	/*获取本次误差和上次误差*/
	p->error1 = p->error0;					//获取上次误差
	p->error0 = p->target - p->actual;		//获取本次误差，目标值减实际值，即为误差值


	/*外环误差积分（累加）*/
	if (p->ki != 0)
	{
		p->errorInt += p->error0;
		/*积分抗饱和限幅*/
		if (p->errorInt > p->outmax)  p->errorInt = p->outmax;
		if (p->errorInt < p->outmin)  p->errorInt = p->outmin;
	}
	else
	{
		p->errorInt = 0;
	}

	/*PID计算*/
	/*使用标准位置式PID公式*/
	p->out = p->kp * p->error0
		   + p->ki * p->errorInt
		   + p->kd * (p->error0 - p->error1);

	/*输出限幅*/
	if (p->out > p->outmax) {p->out = p->outmax;}
	if (p->out < p->outmin) {p->out = p->outmin;}
}


