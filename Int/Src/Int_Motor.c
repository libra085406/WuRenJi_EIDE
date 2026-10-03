#include "Int_Motor.h"

// 电机的句柄结构体初始化
Motor_HandleTypeDef motor_left_top = {&htim3, TIM_CHANNEL_1};     // 电机 M1句柄初始化
Motor_HandleTypeDef motor_right_top = {&htim2, TIM_CHANNEL_2};    // 电机 M2句柄初始化
Motor_HandleTypeDef motor_left_bottom = {&htim4, TIM_CHANNEL_4};  // 电机 M3句柄初始化
Motor_HandleTypeDef motor_right_bottom = {&htim1, TIM_CHANNEL_3}; // 电机 M4句柄初始化

void Int_Motor_Init(void)
{
    HAL_TIM_PWM_Start(motor_left_top.htim, motor_left_top.Channel);         // 开启电机 M1
    HAL_TIM_PWM_Start(motor_right_top.htim, motor_right_top.Channel);       // 开启电机 M2
    HAL_TIM_PWM_Start(motor_left_bottom.htim, motor_left_bottom.Channel);   // 开启电机 M3
    HAL_TIM_PWM_Start(motor_right_bottom.htim, motor_right_bottom.Channel); // 开启电机 M4
}

void Int_Motor_UpdateSpeed(Motor_HandleTypeDef *motorhandle)
{
    __HAL_TIM_SET_COMPARE(motorhandle->htim, motorhandle->Channel, motorhandle->speed); // 更新电机速度
}
