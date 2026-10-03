#ifndef __INT_MOTOR_H__
#define __INT_MOTOR_H__

#include "tim.h"

/* -------- 句柄结构化封装 -------- */
typedef struct
{
    TIM_HandleTypeDef *htim;
    uint16_t Channel;
    uint16_t speed;
} Motor_HandleTypeDef;

/* -------- 句柄结构化变量 -------- */
extern Motor_HandleTypeDef motor_left_top;     // 电机 M1 句柄
extern Motor_HandleTypeDef motor_right_top;    // 电机 M2 句柄
extern Motor_HandleTypeDef motor_left_bottom;  // 电机 M3 句柄
extern Motor_HandleTypeDef motor_right_bottom; // 电机 M4 句柄

/* -------- 函数声明 -------- */
void Int_Motor_Init(void);
void Int_Motor_UpdateSpeed(Motor_HandleTypeDef *motorhandle);

#endif /* __INT_MOTOR_H__ */
