/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
#include "rgb_led.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
Button_t myButton;
Button_t myButton2;
RGB_LED_t my_led;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t on_mo_time = 20 * 1000 ; // 上次按键扫描时间

uint8_t mo_long_state = 0; // 按键状态

void Button_LongCallback()
{

    mo_long_state = 1;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // 打开monitor
    RGB_LED_StartWhiteBreath(&my_led, 2000);// 启动白色呼吸灯效果
}

void Button_ClickCallback(uint8_t count)
{
    // 按键点击回调函数
    // 这里可以添加按键点击后的处理逻辑
    if (count == 1) 
    {

    } 
}
void Button_InactiveCallback()
{

    if (mo_long_state ==1)
    {
         // 无活动回调函数
        // 这里可以添加无活动后的处理逻辑
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // 关闭monitor 
        RGB_LED_StartFlash(&my_led, 255, 0, 0, 200, 200);
        mo_long_state = 0; // 重置状态
    }
    
}
void Button_LongPressReleaseCallback()
{
    // 长按抬起回调函数
    // 这里可以添加长按抬起后的处理逻辑
}



void Button2_longpress_handler() 
{
    // 按下时执行的代码...
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); // 打开monitor
    RGB_LED_StartFlash(&my_led, 0, 0, 255, 400, 400);
}

void Button2_release_handler() 
{
    // 抬起时执行的代码...
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET); // 关闭monitor 
    RGB_LED_StartFlash(&my_led, 255, 0, 0, 200, 200);
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM17_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  Button_Init(&myButton, GPIOA, GPIO_PIN_3); // 初始化按键，连接到PC3

  Button_SetShortPressCallback(&myButton, Button_ClickCallback);
  Button_SetLongPressCallback(&myButton, Button_LongCallback); // 设置长按回调函数，如果不需要可以传NULL
  Button_SetInactiveCallback(&myButton, Button_InactiveCallback, 5000); // 设置无活动回调函数，如果不需要可以传NULL和0
  Button_SetLongPressReleaseCallback(&myButton, Button_LongPressReleaseCallback); // 设置长按抬起回调函数，如果不需要可以传NULL

  Button_Init(&myButton2, GPIOA, GPIO_PIN_4); // 初始化按键，连接到PC3

  Button_SetPressCallback(&myButton2, Button2_longpress_handler);
  Button_SetReleaseCallback(&myButton2, Button2_release_handler);


  RGB_LED_Init(&my_led, LED_CONNECTION_COMMON_ANODE, 
              &htim1, TIM_CHANNEL_2,
              &htim1, TIM_CHANNEL_3,
              &htim1, TIM_CHANNEL_4);

  RGB_LED_StartWhiteBreath(&my_led, 2000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    
    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_3)
  {
    // 执行中断响应逻辑，例如翻转LED
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM17)
    {
        // 每 1ms 进一次，可用于执行按键扫描等任务
        Button_Scan(&myButton);
        Button_Scan(&myButton2);
        RGB_LED_Update(&my_led);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
