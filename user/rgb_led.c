/* === C代码文件: rgb_led.c === */
#include "rgb_led.h"
#include <math.h> // 用于呼吸效果的sin函数

// 内部辅助函数，直接控制GPIO电平
static void _set_pin(GPIO_TypeDef *port, uint16_t pin, bool on) {
    // 假设高电平点亮LED
    HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// 内部辅助函数，根据颜色值设置所有引脚
static void _set_raw_color(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b) {
    _set_pin(led->r_port, led->r_pin, r > 0);
    _set_pin(led->g_port, led->g_pin, g > 0);
    _set_pin(led->b_port, led->b_pin, b > 0);
    led->current_state_on = (r > 0 || g > 0 || b > 0);
}

void RGB_LED_Init(RGB_LED_t *led, GPIO_TypeDef *r_port, uint16_t r_pin,
                  GPIO_TypeDef *g_port, uint16_t g_pin,
                  GPIO_TypeDef *b_port, uint16_t b_pin) {
    led->r_port = r_port;
    led->r_pin = r_pin;
    led->g_port = g_port;
    led->g_pin = g_pin;
    led->b_port = b_port;
    led->b_pin = b_pin;

    // 注意：GPIO的初始化应在外部完成，例如在CubeMX生成的 `MX_GPIO_Init()` 中。
    // 这里只初始化软件状态
    RGB_LED_Off(led);
}

void RGB_LED_Off(RGB_LED_t *led) {
    led->mode = LED_MODE_OFF;
    _set_raw_color(led, 0, 0, 0);
}

void RGB_LED_SetStaticColor(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b) {
    led->mode = LED_MODE_STATIC;
    _set_raw_color(led, r, g, b);
}

void RGB_LED_StartWhiteBreath(RGB_LED_t *led, uint32_t period_ms) {
    led->mode = LED_MODE_BREATH;
    led->breath_period = period_ms > 0 ? period_ms : 2000; // 防止周期为0
    led->timer = 0;
    led->pwm_counter = 0;
}

void RGB_LED_StartFlash(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b,
                        uint32_t on_time_ms, uint32_t off_time_ms) {
    led->mode = LED_MODE_FLASH;
    led->flash_r = r;
    led->flash_g = g;
    led->flash_b = b;
    led->flash_on_time = on_time_ms;
    led->flash_off_time = off_time_ms;
    led->timer = HAL_GetTick(); // 使用HAL_GetTick来标记开始时间
}

void RGB_LED_Update(RGB_LED_t *led) {
    // 软件PWM计数器，每次调用都自增，实现占空比控制
    // 它的循环速度决定了PWM频率
    led->pwm_counter = (led->pwm_counter + 1) % 100;

    switch (led->mode) {
        case LED_MODE_BREATH: {
            // 使用 sinf 函数创建一个平滑的亮度曲线 (0.0 to 1.0)
            // M_PI 是在 math.h 中定义的 PI
            float brightness = (sinf(2.0f * M_PI * led->timer / led->breath_period) + 1.0f) / 2.0f;
            
            // 将亮度值 (0.0-1.0) 转换为软件PWM的阈值 (0-99)
            uint8_t threshold = (uint8_t)(brightness * 100.0f);

            // 当计数器小于阈值时，点亮LED
            bool state = (led->pwm_counter < threshold);
            
            // 白色 = R+G+B
            _set_pin(led->r_port, led->r_pin, state);
            _set_pin(led->g_port, led->g_pin, state);
            _set_pin(led->b_port, led->b_pin, state);

            // 增加时间戳，因为此函数被假定为每1ms调用一次
            led->timer++;
            if (led->timer >= led->breath_period) {
                led->timer = 0; // 一个周期结束，重置计时器
            }
            break;
        }

        case LED_MODE_FLASH: {
            uint32_t elapsed_time = HAL_GetTick() - led->timer;
            uint32_t total_period = led->flash_on_time + led->flash_off_time;

            if (total_period == 0) { // 防止除以零
                RGB_LED_Off(led);
                break;
            }

            bool should_be_on = (elapsed_time % total_period) < led->flash_on_time;

            if (should_be_on != led->current_state_on) { // 仅在状态变化时更新GPIO
                if (should_be_on) {
                    _set_raw_color(led, led->flash_r, led->flash_g, led->flash_b);
                } else {
                    _set_raw_color(led, 0, 0, 0);
                }
            }
            break;
        }

        case LED_MODE_STATIC:
        case LED_MODE_OFF:
        default:
            // 在这些模式下，Update函数无需做任何事
            // 状态已在 SetStaticColor 或 Off 函数中设置
            break;
    }
}
