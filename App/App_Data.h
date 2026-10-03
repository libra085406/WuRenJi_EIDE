#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "Com_Types.h"
#include "Int_SI24R1.h"

#define MY_CRC_KEY2 0x08
#define MY_CRC_KEY1 0x54
#define MY_CRC_KEY0 0x06

extern Com_Throttle_Status_e throttle_status;

/* -------- 函数声明 -------- */
DATA_Valid_e App_Data_Receive(RcData_t *rcData);

Com_Throttle_Status_e App_Data_Unlock(RcData_t *rcData);

void App_Data_ProcessDroneStatus(RcData_t *rcData, COM_DRONE_Status_e *droneStatus, COM_RC_Status_e rcStatus);

#endif /* __APP_DATA_H__ */
