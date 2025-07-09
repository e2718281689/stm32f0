/* === C/C++ Header代码文件: rgb_led.h === */
#ifndef __RGB_LED_H
#define __RGB_LED_H

#include "stm32f0xx_hal.h" // 根据您的MCU系列修改
#include <stdint.h>
#include <stdbool.h>

// --- 颜色定义 (保持不变) ---
#define COLOR_RED       255, 0, 0
#define COLOR_GREEN     0, 255, 0
#define COLOR_BLUE      0, 0, 255
#define COLOR_YELLOW    255, 255, 0
#define COLOR_CYAN      0, 255, 255
#define COLOR_MAGENTA   255, 0, 255
#define COLOR_WHITE     255, 255, 255
#define COLOR_OFF       0, 0, 0

// --- LED连接方式枚举 (新增) ---
typedef enum {
    LED_CONNECTION_COMMON_CATHODE, // 共阴极 (高电平点亮, PWM占空比越大越亮)
    LED_CONNECTION_COMMON_ANODE    // 共阳极 (低电平点亮, PWM占空比越大越暗)
} LED_Connection_t;

// --- LED模式枚举 (保持不变) ---
typedef enum {
    LED_MODE_OFF,
    LED_MODE_STATIC,
    LED_MODE_BREATH,
    LED_MODE_FLASH
} LED_Mode_t;


// --- RGB LED 结构体 (已修改) ---
typedef struct {
    // 硬件PWM配置
    TIM_HandleTypeDef *htim_r;    // R通道的定时器句柄
    uint32_t           channel_r; // R通道的定时器通道 (例如: TIM_CHANNEL_1)
    TIM_HandleTypeDef *htim_g;    // G通道的定时器句柄
    uint32_t           channel_g; // G通道的定时器通道
    TIM_HandleTypeDef *htim_b;    // B通道的定时器句柄
    uint32_t           channel_b; // B通道的定时器通道
    
    // LED连接方式
    LED_Connection_t connection_type; 

    // 当前模式
    LED_Mode_t mode;

    // 动态效果的状态参数
    uint8_t  target_r;    // 闪烁模式的目标颜色
    uint8_t  target_g;
    uint8_t  target_b;
    uint32_t timer_start; // 通用计时器起点
    uint32_t period;      // 呼吸或闪烁的总周期
    uint32_t on_time;     // 闪烁的亮灯时间

} RGB_LED_t;

/**
 * @brief 初始化RGB LED的硬件PWM通道和内部状态
 * @param led 指向RGB_LED_t结构体的指针
 * @param connection LED的连接方式 (共阴/共阳)
 * @param htim_r R通道的定时器句柄
 * @param channel_r R通道的定时器通道
 * @param htim_g G通道的定时器句柄
 * @param channel_g G通道的定时器通道
 * @param htim_b B通道的定时器句柄
 * @param channel_b B通道的定时器通道
 * @note  三个通道可以使用同一个定时器的不同通道，也可以使用不同定时器。
 *        定时器和引脚的初始化应在外部完成 (例如在CubeMX中配置)。
 */
void RGB_LED_Init(RGB_LED_t *led, LED_Connection_t connection,
                  TIM_HandleTypeDef *htim_r, uint32_t channel_r,
                  TIM_HandleTypeDef *htim_g, uint32_t channel_g,
                  TIM_HandleTypeDef *htim_b, uint32_t channel_b);

/**
 * @brief 设置LED为常亮颜色
 * @param led 指向RGB_LED_t结构体的指针
 * @param r, g, b 8位的RGB颜色值 (0-255)
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
 * @note  此函数不再需要高频调用。放在主循环的 while(1) 中即可。
 *        例如每10-20ms调用一次，足以保证动画平滑。
 */
void RGB_LED_Update(RGB_LED_t *led);

#endif
