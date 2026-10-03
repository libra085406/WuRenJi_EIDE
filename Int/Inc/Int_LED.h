#ifndef __INT_LED_H__
#define __INT_LED_H__

#include "main.h"

/* -------- 句柄结构化封装 -------- */
typedef struct
{
    GPIO_TypeDef *gpiox; // LED所在的GPIO端口
    uint16_t gpio_pin;   // LED所在的GPIO引脚
} LED_HandleTypeDef;


/* -------- 句柄结构化变量 -------- */
extern LED_HandleTypeDef led_left_top;     // LED1句柄
extern LED_HandleTypeDef led_right_top;    // LED2句柄
extern LED_HandleTypeDef led_right_bottom; // LED3句柄
extern LED_HandleTypeDef led_left_bottom;  // LED4句柄


/* -------- 函数声明 -------- */
void Int_LED_ON(LED_HandleTypeDef *ledHandle);
void Int_LED_OFF(LED_HandleTypeDef *ledHandle);
void Int_LED_Toggle(LED_HandleTypeDef *ledHandle);

#endif /* __INT_LED_H__ */
