/* === C/C++ Header代码文件: button.h (修改后) === */
#ifndef __BUTTON_H
#define __BUTTON_H

#include "stm32f0xx_hal.h"
#include <stdint.h>

// 按键结构体
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;

    uint8_t last_level;
    uint32_t press_time;
    uint8_t long_press_triggered;

    // 无活动检测相关成员 (保留)
    uint32_t inactive_time;       // 无活动超时时间 (ms)
    uint32_t inactive_timer;      // 无活动计时器
    uint8_t  inactive_triggered;  // 无活动回调已触发标志

    // --- 修改后的回调函数 ---
    void (*on_short_press)(void);        // 短按回调
    void (*on_long_press)(void);         // 长按回调
    void (*on_long_press_release)(void); // 长按后抬起回调
    void (*on_inactive)(void);           // 无活动回调
} Button_t;

void Button_Init(Button_t *btn, GPIO_TypeDef *port, uint16_t pin);

// --- 修改后的回调设置函数 ---
void Button_SetShortPressCallback(Button_t *btn, void (*callback)(void));
void Button_SetLongPressCallback(Button_t *btn, void (*callback)(void));
void Button_SetLongPressReleaseCallback(Button_t *btn, void (*callback)(void));
void Button_SetInactiveCallback(Button_t *btn, void (*callback)(void), uint32_t time);

void Button_Scan(Button_t *btn);     // 建议每 10ms 调用一次

#endif
