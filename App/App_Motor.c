#include "App_Motor.h"

/* ----------------- 全局变量Start ----------------- */
static int16_t last_gyro_x_out = 0; // X轴陀螺仪上一次输出
static int16_t last_gyro_y_out = 0; // Y轴陀螺仪上一次输出
static int16_t last_gyro_z_out = 0; // Z轴陀螺仪上一次输出

PID_Struct pitch_outer_pid = {.kp = 5.0f, .ki = 0.0f, .kd = 0.0f, .dt = 0.1f};    // 俯仰外环PID参数
PID_Struct pitch_inner_pid = {.kp = -2.0f, .ki = 0.0f, .kd = -0.09f, .dt = 0.1f}; // 俯仰内环PID参数
PID_Struct roll_outer_pid = {.kp = 5.0f, .ki = 0.0f, .kd = 0.0f, .dt = 0.1f};     // 横滚外环PID参数
PID_Struct roll_inner_pid = {.kp = 2.0f, .ki = 0.0f, .kd = 0.09f, .dt = 0.1f};    // 横滚内环PID参数
PID_Struct yaw_outer_pid = {.kp = 2.0f, .ki = 0.0f, .kd = 0.0f, .dt = 0.1f};      // 偏航外环PID参数
PID_Struct yaw_inner_pid = {.kp = 4.0f, .ki = 0.0f, .kd = 0.0f, .dt = 0.1f};      // 偏航内环PID参数
/* ----------------- 全局变量End ----------------- */

void App_Motor_GetAG_ByFilter(AG_STRUCT_t *agStruct)
{
    Int_MPU6050_ReadDataByCalibration(&agStruct->accel, &agStruct->gyro);

    // 套用滤波器，对角速度进行一阶低通滤波
    agStruct->gyro.x = Com_LPF(last_gyro_x_out, agStruct->gyro.x, 0.85);
    agStruct->gyro.y = Com_LPF(last_gyro_y_out, agStruct->gyro.y, 0.85);
    agStruct->gyro.z = Com_LPF(last_gyro_z_out, agStruct->gyro.z, 0.85);

    // 更新上一次输出
    last_gyro_x_out = agStruct->gyro.x;
    last_gyro_y_out = agStruct->gyro.y;
    last_gyro_z_out = agStruct->gyro.z;

    // 对加速度进行卡尔曼滤波
    agStruct->accel.x = Common_Filter_KalmanFilter(&kfs[0], agStruct->accel.x); // 对X轴加速度进行卡尔曼滤波
    agStruct->accel.y = Common_Filter_KalmanFilter(&kfs[1], agStruct->accel.y); // 对Y轴加速度进行卡尔曼滤波
    agStruct->accel.z = Common_Filter_KalmanFilter(&kfs[2], agStruct->accel.z); // 对Z轴加速度进行卡尔曼滤波
}

void App_Motor_RunPID(RcData_t *rcdata, EulerAngle_Struct *eulerAngle, AG_STRUCT_t *agStruct)
{
    // 1. 设置外环期望值
    pitch_outer_pid.desire = (rcdata->pitch - 500) * 0.03;
    roll_outer_pid.desire = (rcdata->roll - 500) * 0.03;
    yaw_outer_pid.desire = (rcdata->yaw - 500) * 0.03;

    // 2. 设置外环测量值
    pitch_outer_pid.measure = eulerAngle->pitch;
    roll_outer_pid.measure = eulerAngle->roll;
    yaw_outer_pid.measure = eulerAngle->yaw;

    // 3. 设置内环测量值
    pitch_inner_pid.measure = agStruct->gyro.y * 4000.0 / 65535;
    roll_inner_pid.measure = agStruct->gyro.x * 4000.0 / 65535;
    yaw_inner_pid.measure = agStruct->gyro.z * 4000.0 / 65535;

    // 4. 运行串级PID
    Common_PID_Casecade(&pitch_outer_pid, &pitch_inner_pid);
    Common_PID_Casecade(&roll_outer_pid, &roll_inner_pid);
    Common_PID_Casecade(&yaw_outer_pid, &yaw_inner_pid);
}
