#include "App_Data.h"

/* ----------------- 全局变量Start ----------------- */

Com_Throttle_Status_e current_throttle_status = eThrottle_FREE; // 油门状态
static uint8_t rxbuff[15];                                      // 接收缓存
static uint32_t enter_throttle_MAX_tick = 0;                    // 进入MAX状态时的tick
static uint32_t enter_throttle_MIN_tick = 0;                    // 进入MIN状态时的tick

/* ----------------- 全局变量End ----------------- */
DATA_Valid_e App_Data_Receive(RcData_t *rcData)
{
    uint8_t res = Int_SI24R1_RxPacket(rxbuff);

    // 情况1：没有收到新数据包，直接清空缓存返回无效
    if (res != 0)
    {
        memset(rxbuff, 0, sizeof(rxbuff)); // 清空旧数据
        return eData_Invalid;
    }

    // 情况2：收到数据包，校验KEY
    if ((rxbuff[10] == MY_CRC_KEY2) && (rxbuff[11] == MY_CRC_KEY1) && (rxbuff[12] == MY_CRC_KEY0))
    {
        uint16_t crc = 0;
        for (uint8_t i = 0; i < 13; i++)
        {
            crc += rxbuff[i];
        }
        // CRC校验通过
        if (crc == ((rxbuff[13] << 8) | rxbuff[14]))
        {
            rcData->throttle = (rxbuff[0] << 8) | rxbuff[1];
            rcData->yaw = (rxbuff[2] << 8) | rxbuff[3];
            rcData->pitch = (rxbuff[4] << 8) | rxbuff[5];
            rcData->roll = (rxbuff[6] << 8) | rxbuff[7];
            rcData->off = rxbuff[8];
            rcData->hold_height = rxbuff[9];
            return eData_Valid;
        }
    }

    // 走到这里：收到包但KEY/CRC校验失败，清空缓存
    memset(rxbuff, 0, sizeof(rxbuff));
    return eData_Invalid;
}

Com_Throttle_Status_e App_Data_Unlock(RcData_t *rcData)
{
    switch (current_throttle_status)
    {

    case eThrottle_FREE: // 状态一：空闲
        if (rcData->throttle >= 900)
        {
            current_throttle_status = eThrottle_MAX;
            enter_throttle_MAX_tick = xTaskGetTickCount();
        }
        break;

    case eThrottle_MAX: // 状态二：MAX
        if (rcData->throttle < 900)
        {
            if (xTaskGetTickCount() - enter_throttle_MAX_tick >= 1000)
            {
                current_throttle_status = eThrottle_MID;
            }
            else
            {
                current_throttle_status = eThrottle_FREE;
            }
        }
        break;

    case eThrottle_MID: // 状态三：MID
        if (rcData->throttle <= 100)
        {
            current_throttle_status = eThrottle_MIN;
            enter_throttle_MIN_tick = xTaskGetTickCount();
        }
        break;

    case eThrottle_MIN: // 状态四：MIN
        if (rcData->throttle > 100)
        {
            if (xTaskGetTickCount() - enter_throttle_MIN_tick < 1000)
            {
                current_throttle_status = eThrottle_FREE;
            }
        }
        else
        {
            if (xTaskGetTickCount() - enter_throttle_MIN_tick >= 1000)
            {
                current_throttle_status = eThrottle_UNLOCK;
            }
        }
        break;

    case eThrottle_UNLOCK: // 状态五：UNLOCK
        break;
    }
    return current_throttle_status;
}

void App_Data_ProcessDroneStatus(RcData_t *rcData, COM_DRONE_Status_e *droneStatus, COM_RC_Status_e rcStatus)
{
    switch (*droneStatus)
    {
    case eDRONE_Idle:
        if (App_Data_Unlock(rcData) == eThrottle_UNLOCK)
        {
            *droneStatus = eDRONE_Normal;
        }
        break;
    case eDRONE_Normal:
        if (rcStatus == eRC_Connected)
        {
            if (rcData->hold_height == 1)
            {
                *droneStatus = eDRONE_Hold_Height;
            }
        }
        else
        {
            *droneStatus = eDRONE_Error;
        }
        break;
    case eDRONE_Hold_Height:
        if (rcStatus == eRC_Connected)
        {
            if (rcData->hold_height == 0)
            {
                *droneStatus = eDRONE_Normal;
            }
        }
        else
        {
            *droneStatus = eDRONE_Error;
        }
        break;
    case eDRONE_Error:
        if (rcStatus == eRC_Connected)
        {
            *droneStatus = eDRONE_Normal;
        }
        break;
    }
}
