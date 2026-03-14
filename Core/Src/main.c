/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"
#include "max30102.h"
#include "mpu6050.h"
#include "gps.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
MAX30102_Data max30102_data;
MPU6050_Data mpu6050_data;
GPS_Data gps_data;
uint8_t gps_rx_buf;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// USART 接收回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        // 处理 GPS 数据
        GPS_ProcessData(gps_rx_buf);
        
        // 重新启动接收中断
        HAL_UART_Receive_IT(&huart1, &gps_rx_buf, 1);
    }
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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();
  MAX30102_Init();
  
  // 初始化 MPU6050 (使用标准地址 0x68)
  MPU6050_Init();
  
  // 读取并显示 MPU6050 ID
  uint8_t mpu_id = MPU6050_ReadID();
  OLED_Clear();
  OLED_ShowString(0, 0, (uint8_t*)"MPU6050 ID:", 8, 1);
  OLED_ShowNum(80, 0, mpu_id, 2, 8, 1);
  OLED_Refresh();
  HAL_Delay(500);
  
  // 初始化 GPS 模块
  GPS_Init();
  
  // 启动 USART1 接收中断
  HAL_UART_Receive_IT(&huart1, &gps_rx_buf, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 读取 MAX30102 数据
    MAX30102_ReadFIFO(&max30102_data);
    
    // 读取 MPU6050 数据
    MPU6050_ReadData(&mpu6050_data);
    
    // 读取 GPS 数据
    GPS_GetData(&gps_data);
    
    static uint8_t first_display = 1;
    
    if (first_display)
    {
        // 第一次显示时清除屏幕
        OLED_Clear();
        first_display = 0;
        
        // 显示固定的标签
        OLED_ShowString(0, 0, (uint8_t*)"HR:", 8, 1);
        OLED_ShowString(72, 0, (uint8_t*)"SpO2:", 8, 1);
        OLED_ShowString(0, 8, (uint8_t*)"A:", 8, 1);
        OLED_ShowString(0, 16, (uint8_t*)"G:", 8, 1);
        OLED_ShowString(0, 24, (uint8_t*)"GPS:", 8, 1);
        OLED_ShowString(80, 24, (uint8_t*)"Sats:", 8, 1);
        OLED_ShowString(0, 32, (uint8_t*)"Lat:", 8, 1);
        OLED_ShowString(0, 40, (uint8_t*)"Lon:", 8, 1);
        OLED_ShowString(128, 0, (uint8_t*)"%", 8, 1);
    }
    
    // 只更新变化的数据部分
    
    // 第一行：心率和血氧
    if (max30102_data.heart_rate > 0)
    {
        OLED_ShowNum(24, 0, (uint32_t)max30102_data.heart_rate, 3, 8, 1);
        OLED_ShowString(48, 0, (uint8_t*)"BPM", 8, 1);
    }
    else
    {
        OLED_ShowString(24, 0, (uint8_t*)"---", 8, 1);
    }
    
    if (max30102_data.spo2 > 0)
    {
        OLED_ShowNum(104, 0, (uint32_t)max30102_data.spo2, 3, 8, 1);
    }
    else
    {
        OLED_ShowString(104, 0, (uint8_t*)"---", 8, 1);
    }
    
    // 第二行：加速度计
    OLED_ShowNum(16, 8, (uint32_t)(mpu6050_data.accel_x / 100), 2, 8, 1);
    OLED_ShowNum(48, 8, (uint32_t)(mpu6050_data.accel_y / 100), 2, 8, 1);
    OLED_ShowNum(80, 8, (uint32_t)(mpu6050_data.accel_z / 100), 2, 8, 1);
    
    // 第三行：陀螺仪
    OLED_ShowNum(16, 16, (uint32_t)(mpu6050_data.gyro_x / 100), 2, 8, 1);
    OLED_ShowNum(48, 16, (uint32_t)(mpu6050_data.gyro_y / 100), 2, 8, 1);
    OLED_ShowNum(80, 16, (uint32_t)(mpu6050_data.gyro_z / 100), 2, 8, 1);
    
    // 第四行：GPS 状态
    if (gps_data.fix)
        OLED_ShowString(32, 24, (uint8_t*)"Valid", 8, 1);
    else
        OLED_ShowString(32, 24, (uint8_t*)"Invalid", 8, 1);
    OLED_ShowNum(112, 24, gps_data.satellites, 2, 8, 1);
    
    // 第五行：纬度
    if (gps_data.latitude >= 0)
    {
        OLED_ShowString(32, 32, (uint8_t*)"N ", 8, 1);
    }
    else
    {
        OLED_ShowString(32, 32, (uint8_t*)"S ", 8, 1);
        gps_data.latitude = -gps_data.latitude;
    }
    int lat_int = (int)gps_data.latitude;
    int lat_dec = (int)((gps_data.latitude - lat_int) * 10000);
    OLED_ShowNum(48, 32, lat_int, 2, 8, 1);
    OLED_ShowChar(64, 32, '.', 8, 1);
    OLED_ShowNum(72, 32, lat_dec, 4, 8, 1);
    
    // 第六行：经度
    if (gps_data.longitude >= 0)
    {
        OLED_ShowString(32, 40, (uint8_t*)"E ", 8, 1);
    }
    else
    {
        OLED_ShowString(32, 40, (uint8_t*)"W ", 8, 1);
        gps_data.longitude = -gps_data.longitude;
    }
    int lon_int = (int)gps_data.longitude;
    int lon_dec = (int)((gps_data.longitude - lon_int) * 10000);
    OLED_ShowNum(48, 40, lon_int, 3, 8, 1);
    OLED_ShowChar(72, 40, '.', 8, 1);
    OLED_ShowNum(80, 40, lon_dec, 4, 8, 1);
    
    OLED_Refresh();
    HAL_Delay(500);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
