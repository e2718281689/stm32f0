/* === C代码文件: button.c (修改后) === */
#include "button.h"

#define LONG_PRESS_TIME    500  // 长按阈值 (ms)
#define DEBOUNCE_TIME        50  // 消抖时间 (ms)

void Button_Init(Button_t *btn, GPIO_TypeDef *port, uint16_t pin)
{
    btn->port = port;
    btn->pin = pin;
    btn->last_level = 1; // 假设默认是高电平（松开状态）
    btn->press_time = 0;
    btn->long_press_triggered = 0;

    // 回调函数指针初始化
    btn->on_short_press = 0;
    btn->on_long_press = 0;
    btn->on_long_press_release = 0;
    btn->on_inactive = 0;

    // 无活动检测成员初始化
    btn->inactive_time = 0;
    btn->inactive_timer = 0;
    btn->inactive_triggered = 0;
}

void Button_SetShortPressCallback(Button_t *btn, void (*callback)(void))
{
    btn->on_short_press = callback;
}

void Button_SetLongPressCallback(Button_t *btn, void (*callback)(void))
{
    btn->on_long_press = callback;
}

// 新增：设置长按抬起回调函数
void Button_SetLongPressReleaseCallback(Button_t *btn, void (*callback)(void))
{
    btn->on_long_press_release = callback;
}

// 设置无活动回调函数 (保留)
void Button_SetInactiveCallback(Button_t *btn, void (*callback)(void), uint32_t time)
{
    btn->on_inactive = callback;
    btn->inactive_time = time;
}

// 此函数应放在一个定时器中断中，例如每 10ms 调用一次
void Button_Scan(Button_t *btn)
{
    // 假设按键按下为低电平，松开为高电平
    uint8_t current_level = HAL_GPIO_ReadPin(btn->port, btn->pin);

    // --- 1. 按键按下期间的逻辑 ---
    if (current_level == GPIO_PIN_RESET) 
    {
        // 任何按键活动都会重置无活动计时器
        btn->inactive_timer = 0;
        btn->inactive_triggered = 0;

        if (btn->last_level == GPIO_PIN_SET) { // 刚按下的瞬间
            btn->press_time = 0;
            btn->long_press_triggered = 0;
        } else { // 持续按住
            btn->press_time += 10; // 假设每 10ms 调用一次
            
            // 检查是否达到长按时间，且长按事件尚未触发
            if (btn->press_time >= LONG_PRESS_TIME && !btn->long_press_triggered) {
                if (btn->on_long_press) {
                    btn->on_long_press();
                }
                btn->long_press_triggered = 1; // 标记已触发，防止重复调用
            }
        }
    }
    // --- 2. 按键松开期间的逻辑 ---
    else 
    {
        // 刚松开的瞬间
        if (btn->last_level == GPIO_PIN_RESET) {
            // 任何按键活动都会重置无活动计时器
            btn->inactive_timer = 0;
            btn->inactive_triggered = 0;

            // 判断是哪种事件
            if (btn->long_press_triggered) {
                // 如果长按已触发，则这次是“长按后抬起”
                if (btn->on_long_press_release) {
                    btn->on_long_press_release();
                }
            } else if (btn->press_time >= DEBOUNCE_TIME) {
                // 如果长按未触发，且时间超过消抖时间，则是“短按”
                if (btn->on_short_press) {
                    btn->on_short_press();
                }
            }
            // 时间小于 DEBOUNCE_TIME 的抖动将被忽略

            // 重置状态
            btn->press_time = 0;
            btn->long_press_triggered = 0;
        }

        // --- 3. 无活动判断逻辑 (仅在按键松开时计时) ---
        if (btn->on_inactive && btn->inactive_time > 0 && !btn->inactive_triggered) {
            btn->inactive_timer += 10; // 累加无活动时间
            if (btn->inactive_timer >= btn->inactive_time) {
                btn->on_inactive();
                btn->inactive_triggered = 1; // 标记为已触发，防止重复调用
            }
        }
    }

    btn->last_level = current_level;
}
