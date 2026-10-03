#include "Com_PID.h"

/**
 * @description: pid计算
 *  1. 误差 = 测量值 - 期望值
 *  2. result = Kp*误差 + Ki*(误差 * dt) + Kd*(本次误差 - 上次误差)/dt
 * @param {PID_Struct} pid
 * @return {*}
 */
void Common_PID_Cumpute(PID_Struct *pid)
{
    /* 1. 计算误差 */
    float bias = pid->desire - pid->measure; 

    /* 2. 计算比例项 */
    float pValue = pid->kp * bias;

    /* 3. 计算积分项 */
    pid->integral += pid->ki * bias * pid->dt;

    /* 4. 计算微分项 */
    float dValue = pid->kd * (bias - pid->lastBias) / pid->dt;

    /* 5. 计算pid最终结果 */
    pid->result = pValue + pid->integral + dValue;
    
    /* 6. 把这次偏差保存, 供下次计算使用 */
    pid->lastBias = bias;
}

/**
 * @description: 实现串级pid
 *  外环的输出作为内环的输入
 *  比较灵敏的做内环(需要先稳定, 先调)
 *  不够灵敏的做外环(后稳定,后调)
 * @param {PID_Struct} *outer 外环pid
 * @param {PID_Struct} *inner 内环pid
 * @return {*}
 */
void Common_PID_Casecade(PID_Struct *outer, PID_Struct *inner)
{
    /* 1. 先计算外环的PID */
    Common_PID_Cumpute(outer);

    /* 2. 外环的输出结果作为内环的期望值 */
    inner->desire = outer->result;

    /* 3. 再计算内环的PID */
    Common_PID_Cumpute(inner);
}
