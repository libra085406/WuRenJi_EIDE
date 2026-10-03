#include "Int_LED.h"

// LED的句柄结构体初始化
LED_HandleTypeDef led_left_top = {LED1_GPIO_Port, LED1_Pin};     // LED1句柄初始化
LED_HandleTypeDef led_right_top = {LED2_GPIO_Port, LED2_Pin};    // LED2句柄初始化
LED_HandleTypeDef led_right_bottom = {LED3_GPIO_Port, LED3_Pin}; // LED3句柄初始化
LED_HandleTypeDef led_left_bottom = {LED4_GPIO_Port, LED4_Pin};  // LED4句柄初始化

void Int_LED_ON(LED_HandleTypeDef *ledHandle)
{
    HAL_GPIO_WritePin(ledHandle->gpiox, ledHandle->gpio_pin, GPIO_PIN_RESET);
}

void Int_LED_OFF(LED_HandleTypeDef *ledHandle)
{
    HAL_GPIO_WritePin(ledHandle->gpiox, ledHandle->gpio_pin, GPIO_PIN_SET);
}

void Int_LED_Toggle(LED_HandleTypeDef *ledHandle)
{
    HAL_GPIO_TogglePin(ledHandle->gpiox, ledHandle->gpio_pin);
}
