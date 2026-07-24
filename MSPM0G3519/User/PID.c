#include "PID.h"

void PID_Init(PID_t *p)
{
    p->Target = 0.0f;
    p->Actual = 0.0f;
    p->Out = 0.0f;
    p->Error0 = 0.0f;
    p->Error1 = 0.0f;
    p->ErrorInt = 0.0f;
}

void PID_Update(PID_t *p)
{
    p->Error1 = p->Error0;
    p->Error0 = p->Target - p->Actual;

    if (p->Ki != 0.0f) {
        p->ErrorInt += p->Error0;

        if (p->ErrorIntMax > p->ErrorIntMin) {
            if (p->ErrorInt > p->ErrorIntMax) {
                p->ErrorInt = p->ErrorIntMax;
            } else if (p->ErrorInt < p->ErrorIntMin) {
                p->ErrorInt = p->ErrorIntMin;
            }
        }
    } else {
        p->ErrorInt = 0.0f;
    }

    p->Out = p->Kp * p->Error0
           + p->Ki * p->ErrorInt
           + p->Kd * (p->Error0 - p->Error1);

    if (p->Out > p->OutMax) {
        p->Out = p->OutMax;
    } else if (p->Out < p->OutMin) {
        p->Out = p->OutMin;
    }
}
