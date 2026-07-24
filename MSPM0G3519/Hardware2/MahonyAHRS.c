#include "MahonyAHRS.h"
#include "Timer.h"
#include <math.h>
#include <stdint.h>

#define M_PI_F  3.1415926535f
#define Ki      0.001f

static float Kp = 10.0f;

static volatile float q0 = 1.0f;
static volatile float q1 = 0.0f;
static volatile float q2 = 0.0f;
static volatile float q3 = 0.0f;
static volatile float exInt = 0.0f;
static volatile float eyInt = 0.0f;
static volatile float ezInt = 0.0f;
static volatile uint32_t lastUpdate = 0U;

static float gyro_offset[3] = {0.0f, 0.0f, 0.0f};
static double Gyro_fill[3][100];
static double Gyro_total[3];
static double sqrGyro_total[3];
static int GyroinitFlag = 0;
static int GyroCount = 0;
static int CalCount = 0;

typedef union {
    float f;
    uint32_t u;
} FloatWord_t;

/*******************************************************************************
 * 名    称： invSqrt
 * 功    能：快速计算平方根倒数，用于向量归一化。
 * 参    数：x：需要计算的正数。
 * 出    口：float，返回 1/sqrt(x)，输入异常时返回 0。
 *******************************************************************************/
static float invSqrt(float x)
{
    float halfx;
    FloatWord_t data;

    if (x <= 0.0f) {
        return 0.0f;
    }

    halfx = 0.5f * x;
    data.f = x;
    data.u = 0x5f3759dfU - (data.u >> 1);
    data.f = data.f * (1.5f - (halfx * data.f * data.f));

    return data.f;
}

/*******************************************************************************
 * 名    称： calGyroVariance
 * 功    能：使用滑动窗口估计陀螺仪三轴均值和方差，用于静止时零偏校准。
 * 参    数：data 为当前角速度；sqrResult 输出方差；avgResult 输出均值。
 * 出    口：无返回值。
 *******************************************************************************/
static void calGyroVariance(float data[3], float sqrResult[3], float avgResult[3])
{
    int i;

    if (GyroinitFlag == 0) {
        for (i = 0; i < 3; i++) {
            Gyro_fill[i][GyroCount] = data[i];
            Gyro_total[i] += data[i];
            sqrGyro_total[i] += data[i] * data[i];
            sqrResult[i] = 100.0f;
            avgResult[i] = 0.0f;
        }
    } else {
        for (i = 0; i < 3; i++) {
            Gyro_total[i] -= Gyro_fill[i][GyroCount];
            sqrGyro_total[i] -= Gyro_fill[i][GyroCount] * Gyro_fill[i][GyroCount];
            Gyro_fill[i][GyroCount] = data[i];
            Gyro_total[i] += data[i];
            sqrGyro_total[i] += data[i] * data[i];
        }
    }

    GyroCount++;
    if (GyroCount >= 100) {
        GyroCount = 0;
        GyroinitFlag = 1;
        Kp = 0.5f;
    }

    if (GyroinitFlag == 0) {
        return;
    }

    for (i = 0; i < 3; i++) {
        avgResult[i] = (float)(Gyro_total[i] / 100.0);
        sqrResult[i] = (float)((sqrGyro_total[i] - Gyro_total[i] * Gyro_total[i] / 100.0) / 100.0);
    }
}

/*******************************************************************************
 * 名    称： MahonyAHRS_Init
 * 功    能：初始化姿态四元数、积分误差、陀螺仪零偏和时间戳。
 * 参    数：无。
 * 出    口：无返回值。
 *******************************************************************************/
void MahonyAHRS_Init(void)
{
    int axis;
    int index;

    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    exInt = 0.0f;
    eyInt = 0.0f;
    ezInt = 0.0f;
    Kp = 10.0f;

    for (axis = 0; axis < 3; axis++) {
        gyro_offset[axis] = 0.0f;
        Gyro_total[axis] = 0.0;
        sqrGyro_total[axis] = 0.0;
        for (index = 0; index < 100; index++) {
            Gyro_fill[axis][index] = 0.0;
        }
    }

    GyroinitFlag = 0;
    GyroCount = 0;
    CalCount = 0;
    lastUpdate = nowtime;
}

/*******************************************************************************
 * 名    称： MahonyAHRS_Update
 * 功    能：根据陀螺仪角速度和加速度计重力方向更新姿态四元数。
 * 参    数：gx/gy/gz 单位为 dps；ax/ay/az 单位为 mg。
 * 出    口：无返回值。
 *******************************************************************************/
void MahonyAHRS_Update(float gx_dps, float gy_dps, float gz_dps,
                       float ax_mg,  float ay_mg,  float az_mg)
{
    float sqrResult[3];
    float avgResult[3];
    float gyro_sample[3];
    float accel_norm;
    float norm;
    float vx;
    float vy;
    float vz;
    float ex;
    float ey;
    float ez;
    float halfT;
    float tempq0;
    float tempq1;
    float tempq2;
    float tempq3;
    float gx;
    float gy;
    float gz;
    float ax;
    float ay;
    float az;
    float q0q0;
    float q0q1;
    float q0q2;
    float q1q1;
    float q1q3;
    float q2q2;
    float q2q3;
    float q3q3;
    uint32_t now;

    gyro_sample[0] = gx_dps;
    gyro_sample[1] = gy_dps;
    gyro_sample[2] = gz_dps;

    calGyroVariance(gyro_sample, sqrResult, avgResult);
    if ((sqrResult[0] < 0.02f) && (sqrResult[1] < 0.02f) &&
        (sqrResult[2] < 0.02f) && (CalCount >= 99)) {
        gyro_offset[0] = avgResult[0];
        gyro_offset[1] = avgResult[1];
        gyro_offset[2] = avgResult[2];
        exInt = 0.0f;
        eyInt = 0.0f;
        ezInt = 0.0f;
        CalCount = 0;
    } else if (CalCount < 100) {
        CalCount++;
    }

    gx = (gx_dps - gyro_offset[0]) * M_PI_F / 180.0f;
    gy = (gy_dps - gyro_offset[1]) * M_PI_F / 180.0f;
    gz = (gz_dps - gyro_offset[2]) * M_PI_F / 180.0f;

    ax = ax_mg / 1000.0f;
    ay = ay_mg / 1000.0f;
    az = az_mg / 1000.0f;

    now = nowtime;
    halfT = (float)((uint32_t)(now - lastUpdate)) * 0.0005f;
    lastUpdate = now;
    if (halfT <= 0.0f) {
        return;
    }

    accel_norm = ax * ax + ay * ay + az * az;
    if (accel_norm <= 0.0f) {
        return;
    }

    norm = invSqrt(accel_norm);
    if (norm <= 0.0f) {
        return;
    }

    ax *= norm;
    ay *= norm;
    az *= norm;

    q0q0 = q0 * q0;
    q0q1 = q0 * q1;
    q0q2 = q0 * q2;
    q1q1 = q1 * q1;
    q1q3 = q1 * q3;
    q2q2 = q2 * q2;
    q2q3 = q2 * q3;
    q3q3 = q3 * q3;

    vx = 2.0f * (q1q3 - q0q2);
    vy = 2.0f * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    ex = ay * vz - az * vy;
    ey = az * vx - ax * vz;
    ez = ax * vy - ay * vx;

    if ((ex != 0.0f) || (ey != 0.0f) || (ez != 0.0f)) {
        exInt += ex * Ki * halfT;
        eyInt += ey * Ki * halfT;
        ezInt += ez * Ki * halfT;
        gx += Kp * ex + exInt;
        gy += Kp * ey + eyInt;
        gz += Kp * ez + ezInt;
    }

    tempq0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halfT;
    tempq1 = q1 + ( q0 * gx + q2 * gz - q3 * gy) * halfT;
    tempq2 = q2 + ( q0 * gy - q1 * gz + q3 * gx) * halfT;
    tempq3 = q3 + ( q0 * gz + q1 * gy - q2 * gx) * halfT;

    norm = invSqrt(tempq0 * tempq0 + tempq1 * tempq1 +
                   tempq2 * tempq2 + tempq3 * tempq3);
    if (norm <= 0.0f) {
        return;
    }

    q0 = tempq0 * norm;
    q1 = tempq1 * norm;
    q2 = tempq2 * norm;
    q3 = tempq3 * norm;
}

/*******************************************************************************
 * 名    称： MahonyAHRS_GetYawPitchRoll
 * 功    能：把当前四元数姿态转换成 Yaw、Pitch、Roll 三个欧拉角。
 * 参    数：ypr：长度为 3 的输出数组，单位为度。
 * 出    口：无返回值，结果通过 ypr 输出。
 *******************************************************************************/
void MahonyAHRS_GetYawPitchRoll(float ypr[3])
{
    float sinp;

    ypr[0] = -atan2f(2.0f * q1 * q2 + 2.0f * q0 * q3,
                     -2.0f * q2 * q2 - 2.0f * q3 * q3 + 1.0f) * 180.0f / M_PI_F;

    sinp = -2.0f * q1 * q3 + 2.0f * q0 * q2;
    if (sinp > 1.0f) {
        sinp = 1.0f;
    }
    if (sinp < -1.0f) {
        sinp = -1.0f;
    }

    ypr[1] = -asinf(sinp) * 180.0f / M_PI_F;
    ypr[2] = atan2f(2.0f * q2 * q3 + 2.0f * q0 * q1,
                    -2.0f * q1 * q1 - 2.0f * q2 * q2 + 1.0f) * 180.0f / M_PI_F;
}
