#ifndef __APP_MOTOR_H__
#define __APP_MOTOR_H__

#include "Com_Filter.h"
#include "Com_Types.h"
#include "Com_debug.h"
#include "Com_PID.h"
#include "Int_MPU6050.h"

extern PID_Struct pitch_inner_pid; // 俯仰内环
extern PID_Struct pitch_outer_pid; // 俯仰外环
extern PID_Struct roll_inner_pid;  // 横滚内环
extern PID_Struct roll_outer_pid;  // 横滚外环
extern PID_Struct yaw_inner_pid;   // 偏航内环
extern PID_Struct yaw_outer_pid;   // 偏航外环

void App_Motor_GetAG_ByFilter(AG_STRUCT_t *agStruct);

void App_Motor_RunPID(RcData_t *rcdata, EulerAngle_Struct *eulerAngle, AG_STRUCT_t *agStruct);

#endif /* __APP_MOTOR_H__ */
