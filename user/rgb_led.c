/* === C代码文件: rgb_led.c === */
#include "rgb_led.h"
#include <math.h> // 用于sinf函数

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- 内部辅助函数 ---

/**
 * @brief 根据占空比设置引脚电平 (假设共阴极LED，高电平点亮)
 * @param port GPIO端口
 * @param pin  GPIO引脚
 * @param duty 占空比 (0-SW_PWM_RESOLUTION)
 * @param counter 当前PWM计数器值
 */
static void _set_pin_by_pwm(GPIO_TypeDef *port, uint16_t pin, uint8_t duty, uint8_t counter)
{
    if (counter < duty) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); // 亮
    } else {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET); // 灭
    }
}

/**
 * @brief 将8位颜色值(0-255)转换为PWM占空比(0-SW_PWM_RESOLUTION)
 */
static inline uint8_t _map_color_to_duty(uint8_t color_val)
{
    return (uint8_t)(((uint32_t)color_val * SW_PWM_RESOLUTION) / 255);
}


// --- 公共函数实现 ---

void RGB_LED_Init(RGB_LED_t *led, GPIO_TypeDef *r_port, uint16_t r_pin,
                  GPIO_TypeDef *g_port, uint16_t g_pin,
                  GPIO_TypeDef *b_port, uint16_t b_pin)
{
    led->r_port = r_port;
    led->r_pin = r_pin;
    led->g_port = g_port;
    led->g_pin = g_pin;
    led->b_port = b_port;
    led->b_pin = b_pin;

    // 注意: GPIO引脚的初始化应该在外部完成 (例如在CubeMX生成的 MX_GPIO_Init 中)
    // 这里只初始化软件状态
    RGB_LED_Off(led);
}

void RGB_LED_SetStaticColor(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b)
{
    led->mode = LED_MODE_STATIC;
    led->r_duty = _map_color_to_duty(r);
    led->g_duty = _map_color_to_duty(g);
    led->b_duty = _map_color_to_duty(b);
}

void RGB_LED_Off(RGB_LED_t *led)
{
    led->mode = LED_MODE_OFF;
    led->r_duty = 0;
    led->g_duty = 0;
    led->b_duty = 0;
    // 立即关闭引脚
    HAL_GPIO_WritePin(led->r_port, led->r_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(led->g_port, led->g_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(led->b_port, led->b_pin, GPIO_PIN_RESET);
}

void RGB_LED_StartWhiteBreath(RGB_LED_t *led, uint32_t period_ms)
{
    led->mode = LED_MODE_BREATH;
    led->period = period_ms;
    led->timer_start = HAL_GetTick();
}

void RGB_LED_StartFlash(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b,
                       uint32_t on_time_ms, uint32_t off_time_ms)
{
    led->mode = LED_MODE_FLASH;
    // 预先计算好亮灯时的占空比
    led->r_duty = _map_color_to_duty(r);
    led->g_duty = _map_color_to_duty(g);
    led->b_duty = _map_color_to_duty(b);
    
    led->on_time = on_time_ms;
    led->period = on_time_ms + off_time_ms;
    led->timer_start = HAL_GetTick();
}

void RGB_LED_Update(RGB_LED_t *led)
{
    uint32_t elapsed_time = HAL_GetTick() - led->timer_start;
    uint8_t r_current_duty = 0, g_current_duty = 0, b_current_duty = 0;

    switch (led->mode)
    {
        case LED_MODE_OFF:
            // 占空比已在RGB_LED_Off中被设为0
            break;

        case LED_MODE_STATIC:
            // 占空比已在RGB_LED_SetStaticColor中设置好
            r_current_duty = led->r_duty;
            g_current_duty = led->g_duty;
            b_current_duty = led->b_duty;
            break;

        case LED_MODE_BREATH:
        {
            // 使用正弦函数生成平滑的亮度曲线
            // y = (sin(x) + 1) / 2  -- 将-1~1范围映射到0~1
            float angle = 2.0f * M_PI * (float)(elapsed_time % led->period) / (float)led->period;
            float brightness_factor = (sinf(angle - M_PI / 2.0f) + 1.0f) / 2.0f;
            uint8_t duty = (uint8_t)(brightness_factor * SW_PWM_RESOLUTION);
            
            // 白色呼吸灯，所有通道使用相同占空比
            r_current_duty = duty;
            g_current_duty = duty;
            b_current_duty = duty;
            break;
        }

        case LED_MODE_FLASH:
        {
            if ((elapsed_time % led->period) < led->on_time) {
                // 在亮灯时间内，使用预设的颜色占空比
                r_current_duty = led->r_duty;
                g_current_duty = led->g_duty;
                b_current_duty = led->b_duty;
            } else {
                // 在灭灯时间内，占空比为0
                // r_current_duty, g_current_duty, b_current_duty 默认为0
            }
            break;
        }
    }

    // --- 软件PWM核心 ---
    // 递增PWM计数器
    led->pwm_counter++;
    if (led->pwm_counter >= SW_PWM_RESOLUTION) {
        led->pwm_counter = 0;
    }

    // 根据当前占空比和计数器来设置引脚电平
    _set_pin_by_pwm(led->r_port, led->r_pin, r_current_duty, led->pwm_counter);
    _set_pin_by_pwm(led->g_port, led->g_pin, g_current_duty, led->pwm_counter);
    _set_pin_by_pwm(led->b_port, led->b_pin, b_current_duty, led->pwm_counter);
}
