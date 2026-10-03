#ifndef __COM_TYPES_H__
#define __COM_TYPES_H__

#include "main.h"

typedef enum
{
  eRC_Unconnected, // 遥控器未连接
  eRC_Connected,   // 遥控器已连接
} COM_RC_Status_e;

typedef enum
{
  eDRONE_Idle,        // 空闲待机状态
  eDRONE_Normal,      // 正常飞行状态
  eDRONE_Hold_Height, // 定高状态
  eDRONE_Error        // 错误状态
} COM_DRONE_Status_e;

typedef enum
{
  eData_Valid,  // 数据有效
  eData_Invalid // 数据无效
} DATA_Valid_e;

typedef struct
{
  int16_t throttle;    // 油门
  int16_t yaw;         // 偏航
  int16_t pitch;       // 俯仰
  int16_t roll;        // 横滚
  uint8_t off;         // 关机信号位
  uint8_t hold_height; // 定高信号位
} RcData_t;

typedef enum
{
  eThrottle_FREE,   // 油门基础状态（上锁）
  eThrottle_MAX,    // 油门过高 ---> 解锁状态1
  eThrottle_MID,    // 油门适中 ---> 解锁状态2
  eThrottle_MIN,    // 油门过低 ---> 解锁状态3
  eThrottle_UNLOCK, // 油门最终状态（解锁）
} Com_Throttle_Status_e;

typedef struct
{
  int16_t x; // x轴加速度
  int16_t y; // y轴加速度
  int16_t z; // z轴加速度
} ACCEL_t;

typedef struct
{
  int16_t x; // x轴角速度
  int16_t y; // y轴角速度
  int16_t z; // z轴角速度
} GYRO_t;

typedef struct
{
  ACCEL_t accel; // 加速度结构体
  GYRO_t gyro;   // 角速度结构体
} AG_STRUCT_t;

/* 封装表征飞行器姿态的欧拉角 */
typedef struct
{
  float pitch; /* 俯仰角 */
  float roll;  /* 横滚角 */
  float yaw;   /* 偏航角 */
} EulerAngle_Struct;

/* 通用pid结构体 */
typedef struct
{
  float kp;       /* 比例系数 */
  float ki;       /* 积分系数 */
  float kd;       /* 微分系数 */
  float measure;  /* 测量值 */
  float desire;   /* 期望值 */
  float integral; /* 误差的积分 */
  float lastBias; /* 保存的上次偏差值 */
  float dt;       /* 采样周期 */
  float result;   /*  PID的计算结果 */
} PID_Struct;

#endif /* __COM_TYPES_H__ */
