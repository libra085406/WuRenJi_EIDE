#include "App_Task.h"

/* 全局变量 Start */
COM_RC_Status_e RC_Status = eRC_Unconnected;   // 遥控器连接状态（枚举）
COM_DRONE_Status_e Drone_Status = eDRONE_Idle; // 无人机状态（枚举）
RcData_t RcData;                               // 遥控数据结构体
AG_STRUCT_t AG_STRUCT;                         // 姿态结构体
EulerAngle_Struct EulerAngle;                  // 欧拉角结构体
/* 全局变量 End */

/* ------------ 电源启动任务 ------------ */
TaskHandle_t power_task_handle;
#define POWER_TASK_STACK 128
#define POWER_TASK_PRIORITY 4
void Power_Task_Start(void *pvParameters);

/* ------------ LED任务 ------------ */
TaskHandle_t led_task_handle;
#define LED_TASK_STACK 128
#define LED_TASK_PRIORITY 1
void LED_Task_Start(void *pvParameters);

/* ------------ Motor任务 ------------ */
TaskHandle_t motor_task_handle;
#define MOTOR_TASK_STACK 128
#define MOTOR_TASK_PRIORITY 2
void Motor_Task_Start(void *pvParameters);

/* ------------ 通讯任务 ------------ */
TaskHandle_t comm_task_handle;
#define COMM_TASK_STACK 128
#define COMM_TASK_PRIORITY 3
void Comm_Task_Start(void *pvParameters);

void Start_All_Tasks(void)
{
    /* 0.0 初始化电机 */
    Int_Motor_Init();
    HAL_Delay(400);
    /* 0.1 初始化SI24R1 */
    Int_SI24R1_Init(RX_MODE);
    /* 0.2 初始化MPU6050 */
    Int_MPU6050_Init();

    /* 1. 创建电源任务 */
    xTaskCreate(
        (TaskFunction_t)Power_Task_Start,
        (char *)"Power_Task_Start",
        (configSTACK_DEPTH_TYPE)POWER_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)POWER_TASK_PRIORITY,
        (TaskHandle_t *)&power_task_handle);

    /* 2. 创建LED任务 */
    xTaskCreate(
        (TaskFunction_t)LED_Task_Start,
        (char *)"LED_Task_Start",
        (configSTACK_DEPTH_TYPE)LED_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)LED_TASK_PRIORITY,
        (TaskHandle_t *)&led_task_handle);

    /* 3. 创建Motor任务 */
    xTaskCreate(
        (TaskFunction_t)Motor_Task_Start,
        (char *)"Motor_Task_Start",
        (configSTACK_DEPTH_TYPE)MOTOR_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)MOTOR_TASK_PRIORITY,
        (TaskHandle_t *)&motor_task_handle);

    /* 4. 创建通讯任务 */
    xTaskCreate(
        (TaskFunction_t)Comm_Task_Start,
        (char *)"Comm_Task_Start",
        (configSTACK_DEPTH_TYPE)COMM_TASK_STACK,
        (void *)NULL,
        (UBaseType_t)COMM_TASK_PRIORITY,
        (TaskHandle_t *)&comm_task_handle);

    /* . 启动调度器 */
    vTaskStartScheduler();
}

void Power_Task_Start(void *pvParameters)
{
    debug_printf("Power_Task_Start\r\n");
    while (1)
    {
        uint32_t res = ulTaskNotifyTake(pdTRUE, 10000); // 阻塞等待消息
        if (res == 0)
        {
            // 10秒内未收到遥控关机指令：周期性短按电源芯片按键，给无人机续命，维持整机持续供电
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_RESET); // STM32引脚配置为开漏输出，拉低IO模拟电源按键按下
            vTaskDelay(100);                                                           // IP5305T要求按键电平保持至少30ms，保守延时100ms满足芯片时序要求
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_SET);   // 引脚置高，开漏模式变为高阻浮空，模拟松开电源按键
        }
        else if (res > 0)
        {
            // 收到遥控器下发的关机信号：1秒内连续2次模拟长按电源按键，触发IP5305T整机断电
            /* ==================== 第一次模拟电源按键按下 ==================== */
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_RESET);
            HAL_Delay(100); // 保持100ms满足电源芯片按键时序
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_SET);
            HAL_Delay(250); // 间隔250ms，保证两次按键动作在1秒内完成

            /* ==================== 第二次模拟电源按键按下 ==================== */
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_RESET);
            HAL_Delay(100); // 保持100ms满足电源芯片按键时序
            HAL_GPIO_WritePin(IP5305T_KEY_GPIO_Port, IP5305T_KEY_Pin, GPIO_PIN_SET);
        }
    }
}

void LED_Task_Start(void *pvParameters)
{
    debug_printf("LED_Task_Start\r\n");
    uint32_t last_tick = 0;
    while (1)
    {
        // 根据无人机的遥控状态--->控制LED前两个灯的状态
        switch (RC_Status)
        {
        case eRC_Unconnected:
            // 无人机未连接，关闭前两个灯
            Int_LED_OFF(&led_left_top);
            Int_LED_OFF(&led_right_top);
            break;
        case eRC_Connected:
            // 无人机已连接，常亮前两个灯
            Int_LED_ON(&led_left_top);
            Int_LED_ON(&led_right_top);
            break;
        }

        // 根据无人机的飞行状态--->控制LED后两个灯的状态
        switch (Drone_Status)
        {
        case eDRONE_Idle:
            // 无人机空闲，后两个灯每个隔1秒钟闪烁一次（慢闪烁）
            if (xTaskGetTickCount() - last_tick >= 1000)
            {
                Int_LED_Toggle(&led_left_bottom);
                Int_LED_Toggle(&led_right_bottom);
                last_tick = xTaskGetTickCount();
            }
            break;
        case eDRONE_Normal:
            // 无人机正常飞行，后两个灯每隔100ms闪烁一次（快闪烁）
            if (xTaskGetTickCount() - last_tick >= 100)
            {
                Int_LED_Toggle(&led_left_bottom);
                Int_LED_Toggle(&led_right_bottom);
                last_tick = xTaskGetTickCount();
            }
            break;
        case eDRONE_Hold_Height:
            // 无人机定高，后两个灯常亮
            Int_LED_ON(&led_left_bottom);
            Int_LED_ON(&led_right_bottom);
            break;
        case eDRONE_Error:
            // 无人机错误，后两个灯熄灭
            Int_LED_OFF(&led_left_bottom);
            Int_LED_OFF(&led_right_bottom);
            break;
        }

        // 每隔50ms检查一次状态，更新LED状态
        vTaskDelay(50);
    }
}

void Motor_Task_Start(void *pvParameters)
{
    uint32_t xLastWakeTime = xTaskGetTickCount();
    debug_printf("Motor_Task_Start\r\n");
    while (1)
    {
        App_Motor_GetAG_ByFilter(&AG_STRUCT);
        Common_IMU_GetEulerAngle(&AG_STRUCT, &EulerAngle, 0.01f);

        if (Drone_Status == eDRONE_Idle)
        {
            motor_left_top.speed = 0;     // 电机 M1 速度设置为0
            motor_right_top.speed = 0;    // 电机 M2 速度设置为0
            motor_left_bottom.speed = 0;  // 电机 M3 速度设置为0
            motor_right_bottom.speed = 0; // 电机 M4 速度设置为0
        }
        else if (Drone_Status == eDRONE_Normal)
        {
            App_Motor_RunPID(&RcData, &EulerAngle, &AG_STRUCT);
            motor_left_bottom.speed = LIMIT((LIMIT(RcData.throttle, 0, 800) - pitch_inner_pid.result + roll_inner_pid.result - yaw_inner_pid.result), 0, 1000);
            motor_right_bottom.speed = LIMIT((LIMIT(RcData.throttle, 0, 800) - pitch_inner_pid.result - roll_inner_pid.result + yaw_inner_pid.result), 0, 1000);
            motor_left_top.speed = LIMIT((LIMIT(RcData.throttle, 0, 800) + pitch_inner_pid.result + roll_inner_pid.result + yaw_inner_pid.result), 0, 1000);
            motor_right_top.speed = LIMIT((LIMIT(RcData.throttle, 0, 800) + pitch_inner_pid.result - roll_inner_pid.result - yaw_inner_pid.result), 0, 1000);
        }
        else if (Drone_Status == eDRONE_Hold_Height)
        {
            motor_left_top.speed = RcData.throttle;
            motor_right_top.speed = RcData.throttle;
            motor_left_bottom.speed = RcData.throttle;
            motor_right_bottom.speed = RcData.throttle;
        }
        else if (Drone_Status == eDRONE_Error)
        {
            motor_left_top.speed = 0;
            motor_right_top.speed = 0;
            motor_left_bottom.speed = 0;
            motor_right_bottom.speed = 0;
        }
        if (RcData.throttle < 50)
        {
            motor_left_top.speed = 0;
            motor_right_top.speed = 0;
            motor_left_bottom.speed = 0;
            motor_right_bottom.speed = 0;
        }

        // 更新电机速度
        Int_Motor_UpdateSpeed(&motor_left_top);
        Int_Motor_UpdateSpeed(&motor_right_top);
        Int_Motor_UpdateSpeed(&motor_left_bottom);
        Int_Motor_UpdateSpeed(&motor_right_bottom);
        // 延时10ms
        vTaskDelayUntil(&xLastWakeTime, 10);
    }
}

uint8_t invalidConter = 0;
void Comm_Task_Start(void *pvParameters)
{
    uint32_t xLastWakeTime = xTaskGetTickCount();
    debug_printf("Comm_Task_Start\r\n");
    while (1)
    {
        DATA_Valid_e res = App_Data_Receive(&RcData);
        // 1.处理遥控器连接状态
        if (res == eData_Valid)
        {
            invalidConter = 0;         // 接收到数据，清零无效计数器
            RC_Status = eRC_Connected; // 设置遥控器状态为已连接
        }
        else if (res == eData_Invalid)
        {
            invalidConter++;         // 无效计数器加1
            if (invalidConter >= 50) // 10次无效数据，认为遥控器已断开
            {
                RC_Status = eRC_Unconnected; //  设置遥控器状态为未连接
            }
        }
        // 2.处理关机状态
        if (RcData.off == 1)
        {
            xTaskNotifyGive(power_task_handle); // 发送通知给Power_Task
        }

        // 3.状态机编程-->读取数据，执行状态机，确定状态
        App_Data_ProcessDroneStatus(&RcData, &Drone_Status, RC_Status);

        // 4.每10ms执行一次
        vTaskDelayUntil(&xLastWakeTime, 10);
    }
}
