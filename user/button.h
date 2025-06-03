#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// 按键结构体
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;

    uint8_t last_level;
    uint32_t press_time;
    uint8_t long_press_triggered;

    uint8_t click_count;
    uint32_t click_timer;

    void (*on_click)(uint8_t count);     // 连击回调
    void (*on_long_press)(void);         // 长按回调
} Button_t;

void Button_Init(Button_t *btn, GPIO_TypeDef *port, uint16_t pin);
void Button_SetClickCallback(Button_t *btn, void (*callback)(uint8_t));
void Button_SetLongPressCallback(Button_t *btn, void (*callback)(void));
void Button_Scan(Button_t *btn);     // 每 10ms 调用

#endif
