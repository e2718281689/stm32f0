#include "button.h"

#define LONG_PRESS_TIME    1000  // ms
#define DEBOUNCE_TIME        50  // ms
#define CLICK_TIMEOUT       400  // 连击等待时间（ms）

void Button_Init(Button_t *btn, GPIO_TypeDef *port, uint16_t pin)
{
    btn->port = port;
    btn->pin = pin;
    btn->last_level = 1;
    btn->press_time = 0;
    btn->long_press_triggered = 0;
    btn->click_count = 0;
    btn->click_timer = 0;
    btn->on_click = 0;
    btn->on_long_press = 0;
}

void Button_SetClickCallback(Button_t *btn, void (*callback)(uint8_t))
{
    btn->on_click = callback;
}

void Button_SetLongPressCallback(Button_t *btn, void (*callback)(void))
{
    btn->on_long_press = callback;
}

void Button_Scan(Button_t *btn)
{
    uint8_t current = HAL_GPIO_ReadPin(btn->port, btn->pin);

    if (current == GPIO_PIN_RESET)  // 按下中
    {
        if (btn->last_level == GPIO_PIN_RESET) {
            btn->press_time += 10;

            if (btn->press_time >= LONG_PRESS_TIME && !btn->long_press_triggered) {
                btn->long_press_triggered = 1;
                if (btn->on_long_press) btn->on_long_press();
                btn->click_count = 0;
                btn->click_timer = 0;
            }
        }
    }
    else  // 松开
    {
        if (btn->last_level == GPIO_PIN_RESET) {
            if (btn->press_time >= DEBOUNCE_TIME && btn->press_time < LONG_PRESS_TIME && !btn->long_press_triggered) {
                btn->click_count++;
                btn->click_timer = 0;  // 启动连击计时器
            }
        }

        btn->press_time = 0;
        btn->long_press_triggered = 0;
    }

    // 连击超时判断
    if (btn->click_count > 0) {
        btn->click_timer += 10;
        if (btn->click_timer >= CLICK_TIMEOUT) {
            if (btn->on_click) {
                btn->on_click(btn->click_count);  // 传入点击次数
            }
            btn->click_count = 0;
            btn->click_timer = 0;
        }
    }

    btn->last_level = current;
}
