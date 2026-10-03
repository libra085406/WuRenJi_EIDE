#ifndef __APP_TASK_H__
#define __APP_TASK_H__

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include "main.h"
#include "usart.h"
#include "Com_debug.h"
#include "Com_Types.h"
#include "Int_LED.h"
#include "Int_Motor.h"
#include "Int_SI24R1.h"
#include "Int_MPU6050.h"
#include "App_Data.h"
#include "App_Motor.h"
#include "Com_IMU.h"
#include "Com_PID.h"

#define LIMIT(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

void Start_All_Tasks(void);

#endif /* __APP_TASK_H__ */
