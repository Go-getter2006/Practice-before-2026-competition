#ifndef PID_H
#define PID_H

typedef struct
{
    float Target;
    float Actual;
    float Out;

    float Kp;
    float Ki;
    float Kd;

    float Error0;
    float Error1;
    float ErrorInt;
    float ErrorIntMax;
    float ErrorIntMin;

    float OutMax;
    float OutMin;
} PID_t;

/* 清除 PID 运行状态，不改变增益和限幅参数。 */
void PID_Init(PID_t *p);

/* 根据 Target 和 Actual 更新 Out，调用周期应保持固定。 */
void PID_Update(PID_t *p);

#endif
