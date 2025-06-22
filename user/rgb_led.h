/* === C/C++ Header代码文件: rgb_led.h === */
#ifndef __RGB_LED_H
#define __RGB_LED_H

#include "stm32f0xx_hal.h" // 根据你的MCU型号修改，例如 "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// --- RGB LED 状态定义 ---
typedef enum {
    LED_MODE_OFF,       // 关闭
    LED_MODE_STATIC,    // 静态（常亮）
    LED_MODE_BREATH,    // 呼吸
    LED_MODE_FLASH      // 闪烁
} LED_Mode_t;

// --- RGB LED 结构体 ---
// 存储一个LED灯的所有信息和状态
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

    // 状态变量
    uint32_t timer;                // 通用计时器，用于模式内部计时
    bool current_state_on;         // 当前LED的物理亮灭状态

    // 呼吸模式参数
    uint32_t breath_period;        // 一个完整的呼吸周期 (ms)

    // 闪烁模式参数
    uint8_t flash_r, flash_g, flash_b; // 闪烁的颜色
    uint32_t flash_on_time;        // 闪烁亮起时间 (ms)
    uint32_t flash_off_time;       // 闪烁熄灭时间 (ms)

    // 软件PWM相关 (用于呼吸效果)
    uint8_t pwm_counter;           // 软件PWM计数器 (0-99)

} RGB_LED_t;


/**
 * @brief  初始化RGB LED
 * @param  led       指向LED结构体的指针
 * @param  r_port    R通道的GPIO端口
 * @param  r_pin     R通道的GPIO引脚
 * @param  g_port    G通道的GPIO端口
 * @param  g_pin     G通道的GPIO引脚
 * @param  b_port    B通道的GPIO端口
 * @param  b_pin     B通道的GPIO引脚
 */
void RGB_LED_Init(RGB_LED_t *led, GPIO_TypeDef *r_port, uint16_t r_pin,
                  GPIO_TypeDef *g_port, uint16_t g_pin,
                  GPIO_TypeDef *b_port, uint16_t b_pin);

/**
 * @brief  关闭LED
 * @param  led 指向LED结构体的指针
 */
void RGB_LED_Off(RGB_LED_t *led);

/**
 * @brief  设置LED为静态常亮颜色
 * @param  led 指向LED结构体的指针
 * @param  r   红色分量 (0或255)
 * @param  g   绿色分量 (0或255)
 * @param  b   蓝色分量 (0或255)
 */
void RGB_LED_SetStaticColor(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief  启动白色呼吸灯效果
 * @param  led         指向LED结构体的指针
 * @param  period_ms   一个完整的呼吸周期 (例如 2000ms)
 */
void RGB_LED_StartWhiteBreath(RGB_LED_t *led, uint32_t period_ms);

/**
 * @brief  启动指定颜色的闪烁效果
 * @param  led           指向LED结构体的指针
 * @param  r             红色分量 (0或255)
 * @param  g             绿色分量 (0或255)
 * @param  b             蓝色分量 (0或255)
 * @param  on_time_ms    亮起持续时间 (ms)
 * @param  off_time_ms   熄灭持续时间 (ms)
 */
void RGB_LED_StartFlash(RGB_LED_t *led, uint8_t r, uint8_t g, uint8_t b,
                        uint32_t on_time_ms, uint32_t off_time_ms);

/**
 * @brief  更新LED状态机，必须被周期性调用
 * @note   为了达到较好的呼吸灯效果，此函数应尽可能频繁地调用，
 * **强烈建议放在1ms的SysTick中断中**。
 * @param  led 指向LED结构体的指针
 */
void RGB_LED_Update(RGB_LED_t *led);

#endif
