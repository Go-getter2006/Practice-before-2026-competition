#ifndef INC_PID_H_
#define INC_PID_H_

typedef struct {		//定义PID结构体变量类型
	float target;		//目标值，由用户设定
	float actual;		//实际值，从传感器读取
	float out;			//输出值，作用于执行器

	float kp;			//比例项权重
	float ki;			//积分项权重
	float kd;			//微分项权重

	float error0;		//本次误差
	float error1;		//上次误差
	float errorInt;		//误差积分

	float outmax;		//输出限幅的最大值
	float outmin;		//输出限幅的最小值
} PID_t;

void PID_Init(PID_t *p);
void PID_Update(PID_t *p);

#endif /* INC_PID_H_ */
