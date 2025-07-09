/* === C代码文件: rgb_led.c === */
#include "rgb_led.h"
#include <math.h> // 用于sinf函数

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// --- 内部辅助函数 ---

/**
 * @brief 根据颜色值(0-255)和连接方式设置硬件PWM占空比
 * @param htim      定时器句柄
 * @param channel   定时器通道
 * @param color_val 颜色亮度值 (0-255)
 * @param connection_type LED连接方式 (共阴/共阳)
 */
static void _set_pwm_by_color(TIM_HandleTypeDef *htim, uint32_t channel, 
                              uint8_t color_val, LED_Connection_t connection_type)
{
    // 获取定时器的自动重载值 (ARR)，这就是PWM的最大计数值
    uint32_t max_duty = __HAL_TIM_GET_AUTORELOAD(htim);
    
    // 将 0-255 的颜色值映射到 0-max_duty 的PWM比较值
    uint32_t compare_val = ((uint32_t)color_val * max_duty) / 255;

    // 根据连接方式调整最终的比较值
    if (connection_type == LED_CONNECTION_COMMON_ANODE) {
        // 共阳极，逻辑反转。0%占空比最亮，100%占空比最暗。
        compare_val = max_duty - compare_val;
    }
    
    // 设置PWM比较寄存器值
    __HAL_TIM_SET_COMPARE(htim, channel, compare_val);
}

// --- 公共函数实现 ---

void RGB_LED_Init(RGB_LED_t *led, LED_Connection_t connection,
                  TIM_HandleTypeDef *htim_r, uint32_t channel_r,
                  TIM_HandleTypeDef *htim_g, uint32_t channel_g,
                  TIM_HandleTypeDef *htim_b, uint32_t channel_b)
{
    led->connection_type = connection;
    led->htim_r = htim_r;
    led->channel_r = channel_r;
    led->htim_g = htim_g;
    led->channel_g = channel_g;
    led->htim_b = htim_b;
    led->channel_b = channel_b;

    // 启动所有PWM通道
    HAL_TIM_PWM_Start(led->htim_r, led->channel_r);
    HAL_TIM_PWM_Start(led->htim_g, led->channel_g);
    HAL_TIM_PWM_Start(led->htim_b, led->channel_b);
    
    // 初始化为关闭状态
    RGB_LED_Off(led);
}

void RGB_LED_SetStaticColor(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b)
{
    led->mode = LED_MODE_STATIC;
    _set_pwm_by_color(led->htim_r, led->channel_r, r, led->connection_type);
    _set_pwm_by_color(led->htim_g, led->channel_g, g, led->connection_type);
    _set_pwm_by_color(led->htim_b, led->channel_b, b, led->connection_type);
}

void RGB_LED_Off(RGB_LED_t *led)
{
    led->mode = LED_MODE_OFF;
    // 设置颜色为(0,0,0)即可关闭
    RGB_LED_SetStaticColor(led, 0, 0, 0);
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
    led->target_r = r;
    led->target_g = g;
    led->target_b = b;
    led->on_time = on_time_ms;
    led->period = on_time_ms + off_time_ms;
    led->timer_start = HAL_GetTick();
}

void RGB_LED_Update(RGB_LED_t *led)
{
    // 对于静态和关闭模式，PWM占空比已设定，无需更新
    if (led->mode == LED_MODE_STATIC || led->mode == LED_MODE_OFF) {
        return;
    }

    uint32_t elapsed_time = HAL_GetTick() - led->timer_start;

    switch (led->mode)
    {
        case LED_MODE_BREATH:
        {
            // 使用正弦函数生成平滑的亮度曲线
            // y = (sin(x) + 1) / 2  -- 将-1~1范围映射到0~1
            float angle = 2.0f * M_PI * (float)(elapsed_time % led->period) / (float)led->period;
            float brightness_factor = (sinf(angle - M_PI / 2.0f) + 1.0f) / 2.0f;
            uint8_t color_val = (uint8_t)(brightness_factor * 255.0f);
            
            // 白色呼吸灯，所有通道使用相同亮度
            _set_pwm_by_color(led->htim_r, led->channel_r, color_val, led->connection_type);
            _set_pwm_by_color(led->htim_g, led->channel_g, color_val, led->connection_type);
            _set_pwm_by_color(led->htim_b, led->channel_b, color_val, led->connection_type);
            break;
        }

        case LED_MODE_FLASH:
        {
            if ((elapsed_time % led->period) < led->on_time) {
                // 亮灯时间
                _set_pwm_by_color(led->htim_r, led->channel_r, led->target_r, led->connection_type);
                _set_pwm_by_color(led->htim_g, led->channel_g, led->target_g, led->connection_type);
                _set_pwm_by_color(led->htim_b, led->channel_b, led->target_b, led->connection_type);
            } else {
                // 灭灯时间
                _set_pwm_by_color(led->htim_r, led->channel_r, 0, led->connection_type);
                _set_pwm_by_color(led->htim_g, led->channel_g, 0, led->connection_type);
                _set_pwm_by_color(led->htim_b, led->channel_b, 0, led->connection_type);
            }
            break;
        }

        default:
            break;
    }
}
