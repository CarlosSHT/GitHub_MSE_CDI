/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "lwip.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include "math.h"

#include "udp_client_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef bool bool_t;

//struct header_struct {
//   char     pre[8];
//   uint32_t id;
//   uint16_t N;
//   uint16_t fs ;
//   uint16_t hLength ;
//   char     pos[4];
//} __attribute__ ((packed));
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DAC_VMAX   2480  // 4095 = 3.3V, 666 = 2.0V
#define DAC_VMIN    1241  // 4095 = 3.3V, 356 = 1.0V

#define DAC_VREF	3.3
#define DAC_RES		4096
#define SAMPLES_SIG	1800 // Tiempo en milisegundos
#define SAMPLES_ADC	40 		// Frecuencia muestreo ADC es 200 Hz
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
bool_t sig10hz_run = false;
bool_t dac_updt = false;
uint32_t dac_value = DAC_VMIN;

uint32_t square_vals[SAMPLES_SIG];

uint16_t adc_bufferIN[SAMPLES_ADC*2];
uint16_t adc_bufferOUT[SAMPLES_ADC*2];

uint16_t adc_samplesIN[SAMPLES_ADC];
uint16_t adc_samplesOUT[SAMPLES_ADC];

uint8_t tx_UDPout[SAMPLES_ADC*4];

bool_t sendtest=false;

bool_t rdy_midAD=false;
bool_t rdy_cpltAD=false;
bool_t proc_samples=false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void app_startSignal10Hz(void);
void app_stopSignal10Hz(void);

void app_generateSquare(uint32_t *buff, uint32_t len, uint8_t duty);

//void app_setDACvalue(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_DAC_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_LWIP_Init();
  MX_TIM8_Init();
  MX_TIM3_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
	udpApp_client_Initialization();

	HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, DAC_VMIN);
//	app_generateSquare(square_vals, SAMPLES_SIG, 50);
//	app_startSignal10Hz();

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_bufferIN, SAMPLES_ADC*2);
	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_bufferOUT, SAMPLES_ADC*2);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_TIM_Base_Start_IT(&htim8);
	printf("Hola mundo CDI\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		MX_LWIP_Process();	//!> Proceso de STM32 HAL LWIP utilizado para el manejo del servicio ethernet

		if (dac_updt) {
			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
//			dac_value  = DAC_VMIN + rand() % (DAC_VMAX+1 - DAC_VMIN);
			if (dac_value == DAC_VMIN) dac_value=DAC_VMAX;
			else dac_value = DAC_VMIN;

			HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
			dac_updt = false;
		}

		if (rdy_midAD) {
//			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			memcpy(adc_samplesIN, &adc_bufferIN[0], sizeof(adc_bufferIN) / 2 );
			memcpy(adc_samplesOUT, &adc_bufferOUT[0], sizeof(adc_bufferOUT) / 2 );
			rdy_midAD=false;
			proc_samples=true;
		}

		if (rdy_cpltAD) {
//			HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
			memcpy(adc_samplesIN, &adc_bufferIN[SAMPLES_ADC], sizeof(adc_bufferIN) / 2);
			memcpy(adc_samplesOUT, &adc_bufferOUT[SAMPLES_ADC], sizeof(adc_bufferOUT) / 2);
			rdy_cpltAD=false;
			proc_samples=true;
		}

		if (proc_samples) {
			HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);

			memcpy(&tx_UDPout[0], &adc_samplesIN[0], sizeof(adc_samplesIN));
			memcpy(&tx_UDPout[sizeof(adc_samplesIN)], &adc_samplesOUT[0], sizeof(adc_samplesOUT));

			udpApp_client_send2((char*)&tx_UDPout,sizeof(tx_UDPout));

			proc_samples=false;
		}


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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

void app_startSignal10Hz(void) {
	if (!sig10hz_run) {
		HAL_TIM_Base_Start_IT(&htim8);
//		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, square_vals, SAMPLES_SIG,
//				DAC_ALIGN_12B_R);
		sig10hz_run = true;
	}
}

void app_stopSignal10Hz(void) {
	if (sig10hz_run) {
		HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
		HAL_TIM_Base_Stop(&htim8);
		sig10hz_run = false;
	}
}

//void app_generateSquare(uint32_t *buff, uint32_t len, uint8_t duty) {
//	uint32_t n_low = (uint32_t) (((uint64_t) duty * (uint64_t) len) / 100);
//	uint32_t n_high = len - n_low;
//
//	for (int var = 0; var < n_low; ++var) {
//		buff[var] = (uint32_t) (DAC_VMIN * DAC_RES) / DAC_VREF;
//		;
//	}
//
//	for (int var = n_low; var < n_low + n_high; ++var) {
//		buff[var] = (uint32_t) (DAC_VMAX * DAC_RES) / DAC_VREF;
//		;
//	}
//	printf("N bajo es : %d\n", n_low);
//
//	printf("N alto es : %d\n", n_high);
//}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc1 || hadc == &hadc2) {
		if (!rdy_midAD) rdy_midAD=true;
	}
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc1 || hadc == &hadc2) {
		if (!rdy_cpltAD) rdy_cpltAD=true;
	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Check which version of the timer triggered this callback and toggle LED
  if (htim == &htim8 )
  {
		if (!dac_updt) dac_updt=true;
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
	while (1) {
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
