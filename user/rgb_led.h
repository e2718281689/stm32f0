/* === C/C++ Header代码文件: rgb_led.h === */
#ifndef __RGB_LED_H
#define __RGB_LED_H

#include "stm32f0xx_hal.h" // 根据您的MCU系列修改
#include <stdint.h>
#include <stdbool.h>

// --- 公共配置 ---
#define SW_PWM_RESOLUTION   100  // 软件PWM的分辨率 (0-100)。值越高，亮度过渡越平滑，但要求Update函数调用更频繁。

// --- 颜色定义 ---
#define COLOR_RED       255, 0, 0
#define COLOR_GREEN     0, 255, 0
#define COLOR_BLUE      0, 0, 255
#define COLOR_YELLOW    255, 255, 0
#define COLOR_CYAN      0, 255, 255
#define COLOR_MAGENTA   255, 0, 255
#define COLOR_WHITE     255, 255, 255
#define COLOR_OFF       0, 0, 0


// --- 按键状态枚举 ---
typedef enum {
    LED_MODE_OFF,
    LED_MODE_STATIC,
    LED_MODE_BREATH,
    LED_MODE_FLASH
} LED_Mode_t;


// --- RGB LED 结构体 ---
typedef struct {
    // GPIO 配置
    GPIO_TypeDef *r_port;
    uint16_t r_pin;
    GPIO_TypeDef *g_port;
    uint16_t g_pin;
    GPIO_TypeDef *b_port;
    uint16_t b_pin;

    // 当前模式
    LED_Mode_t mode;

    // 软件PWM相关 (内部使用)
    uint8_t pwm_counter;
    uint8_t r_duty; // 0-SW_PWM_RESOLUTION
    uint8_t g_duty; // 0-SW_PWM_RESOLUTION
    uint8_t b_duty; // 0-SW_PWM_RESOLUTION

    // 状态参数
    uint32_t timer_start; // 通用计时器起点
    uint32_t period;      // 呼吸或闪烁的总周期
    uint32_t on_time;     // 闪烁的亮灯时间

} RGB_LED_t;

/**
 * @brief 初始化RGB LED的GPIO引脚和内部状态
 * @param led 指向RGB_LED_t结构体的指针
 * @param r_port R通道的GPIO端口
 * @param r_pin  R通道的GPIO引脚
 * @param g_port G通道的GPIO端口
 * @param g_pin  G通道的GPIO引脚
 * @param b_port B通道的GPIO端口
 * @param b_pin  B通道的GPIO引脚
 */
void RGB_LED_Init(RGB_LED_t *led, GPIO_TypeDef *r_port, uint16_t r_pin,
                  GPIO_TypeDef *g_port, uint16_t g_pin,
                  GPIO_TypeDef *b_port, uint16_t b_pin);

/**
 * @brief 设置LED为常亮颜色
 * @note  颜色亮度值范围 0-255，会被自动映射到 0-SW_PWM_RESOLUTION
 * @param led 指向RGB_LED_t结构体的指针
 * @param r, g, b 8位的RGB颜色值
 */
void RGB_LED_SetStaticColor(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 关闭LED
 * @param led 指向RGB_LED_t结构体的指针
 */
void RGB_LED_Off(RGB_LED_t *led);

/**
 * @brief 启动白色呼吸灯效果
 * @param led 指向RGB_LED_t结构体的指针
 * @param period_ms 一个完整的呼吸周期时间 (从最暗到最亮再到最暗)
 */
void RGB_LED_StartWhiteBreath(RGB_LED_t *led, uint32_t period_ms);

/**
 * @brief 启动指定颜色的闪烁效果
 * @param led 指向RGB_LED_t结构体的指针
 * @param r, g, b   闪烁的颜色 (0-255)
 * @param on_time_ms  亮灯时间 (ms)
 * @param off_time_ms 灭灯时间 (ms)
 */
void RGB_LED_StartFlash(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b,
                       uint32_t on_time_ms, uint32_t off_time_ms);

/**
 * @brief 更新LED状态，实现动态效果 (呼吸/闪烁)
 * @note  !!! 关键函数 !!!
 * 必须放在一个高频、精确的定时器中断中调用，例如 SysTick 中断。
 * 推荐调用频率: 1ms (1000Hz)
 */
void RGB_LED_Update(RGB_LED_t *led);

#endif
