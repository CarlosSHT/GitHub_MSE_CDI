/*******************************************************************************
* Title                 :   ------
* Filename              :   app.c
* Author                :   Carlos Herrera Trujillo
* Origin Date           :   Jun 8, 2024
* Version               :   x.0.0
* Compiler              :   ------
* Target                :   STM32XXX
* Notes                 :   
*******************************************************************************/

/******************************************************************************
* Private Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Private Includes
*******************************************************************************/
#include "app.h"
#include "samples_vectors.h"
#include "main.h"
#include "usart.h"
#include "lwip.h"
#include <string.h>
#include <stdbool.h>
#include "adc.h"
#include "dac.h"
#include "tim.h"
/******************************************************************************
* Private defines
*******************************************************************************/
#define USE_DAC_BUFFER	1	// Set CubeMX DAC OUT1 Trigger TIMER 8


#define SAMPLES_ADC	40 		// Number samples to send
#define FS_ADC		500
#define DAC_VMAX	2480 	// 4095 = 3.3V, 666 = 2.0V
#define DAC_VMIN	1241	// 4095 = 3.3V, 356 = 1.0V

#define UDPTX_INDEX	0
#define UDPTX_NSIGN	3
/******************************************************************************
* Private Typedefs
*******************************************************************************/
typedef bool bool_t;

typedef struct __attribute__((packed)) _header_struct
{
	char     pre[4];
	uint32_t	id;	// start index
	uint8_t		s;	// quantity of signals
	uint16_t	N;	// samples per signals
	uint16_t	fs;	//frequency sample
	char     pos[4];
}header_struct;
/******************************************************************************
* Private Macros
*******************************************************************************/

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

/******************************************************************************
* Public Variables
*******************************************************************************/


/******************************************************************************
* Private Variables
*******************************************************************************/

header_struct headerUDP={"head",UDPTX_INDEX,UDPTX_NSIGN,SAMPLES_ADC,FS_ADC,"tail"};


uint16_t adc_bufferIN[SAMPLES_ADC*2];		// ADC buffer vector with double size number samples
uint16_t adc_bufferC1[SAMPLES_ADC*2];		// ADC buffer vector with double size number samples
uint16_t adc_bufferC2[SAMPLES_ADC*2];		// ADC buffer vector with double size number samples

uint16_t adc_samplesIN[SAMPLES_ADC];		// Vector with samples to send
uint16_t adc_samplesC1[SAMPLES_ADC];		// Vector with samples to send
uint16_t adc_samplesC2[SAMPLES_ADC];		// Vector with samples to send


bool_t rdy_midAD=false;		//	Flag with ADC buffer half complete TIMER 8
bool_t rdy_cpltAD=false;	//	Flag with ADC buffer full complete TIMER 8
bool_t proc_samples=false;	//	Flag ready to send samples

uint8_t payload_udp[sizeof(headerUDP) + sizeof(adc_samplesIN) + sizeof(adc_samplesC1) + sizeof(adc_samplesC2)];		// PAYLOAD UDP: data samples

uint16_t *dac_buffer =  step_1;
uint32_t dac_bsize = SIZE_STEP_1;
uint16_t dac_buffer_init [20];
uint16_t dac_val;			// Value 12 bits DAC
bool_t dac_updt = false;	// Flag DAC update with TIMER 7

bool_t start_tmrs = false;
/******************************************************************************
* Private Function Prototypes
*******************************************************************************/


/******************************************************************************
* Public Function Prototypes
*******************************************************************************/


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////Definitions/////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/******************************************************************************
* Private Function Definitions
*******************************************************************************/


/******************************************************************************
* Public Function Definitions
*******************************************************************************/
void app_Init(void)
{
	udpclient_Init(DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);

#ifdef USE_DAC_BUFFER

	for (int var = 0; var < 20; ++var) {
		 dac_buffer_init[var]= dac_buffer[0];
		 dac_buffer_init[var]= 993;
	}

	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)dac_buffer_init, 20, DAC_ALIGN_12B_R);
	HAL_TIM_Base_Start_IT(&htim7);

#else
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
#endif
	HAL_TIM_OnePulse_Start_IT(&htim6, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(&htim6);
}

void app_fsm(void)
{
	MX_LWIP_Process();

#ifndef USE_DAC_BUFFER
	if (dac_updt) {
		HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

		if (dac_val == DAC_VMIN)
			dac_val = DAC_VMAX;
		else
			dac_val = DAC_VMIN;

		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val);
		dac_updt = false;
	}
#endif
	if (start_tmrs) {
		start_tmrs = false;
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

//		HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
//		HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)dac_buffer, dac_bsize, DAC_ALIGN_12B_R);

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_bufferIN, SAMPLES_ADC*2);
		HAL_ADC_Start_DMA(&hadc2, (uint32_t*)adc_bufferC1, SAMPLES_ADC*2);
		HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc_bufferC2, SAMPLES_ADC*2);
		HAL_TIM_Base_Start_IT(&htim8);
	}

	if (rdy_midAD) {
		memcpy(adc_samplesIN, &adc_bufferIN[0], sizeof(adc_bufferIN) / 2);
		memcpy(adc_samplesC1, &adc_bufferC1[0], sizeof(adc_bufferC1) / 2);
		memcpy(adc_samplesC2, &adc_bufferC2[0], sizeof(adc_bufferC2) / 2);
		rdy_midAD = false;
		proc_samples = true;
	}

	if (rdy_cpltAD) {
		memcpy(adc_samplesIN, &adc_bufferIN[SAMPLES_ADC], sizeof(adc_bufferIN) / 2);
		memcpy(adc_samplesC1, &adc_bufferC1[SAMPLES_ADC], sizeof(adc_bufferC1) / 2);
		memcpy(adc_samplesC2, &adc_bufferC2[SAMPLES_ADC], sizeof(adc_bufferC2) / 2);
		rdy_cpltAD = false;
		proc_samples = true;
	}

	if (proc_samples) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);


		memcpy(&payload_udp[0], &headerUDP, sizeof(headerUDP));
		memcpy(&payload_udp[sizeof(headerUDP)], &adc_samplesIN[0], sizeof(adc_samplesIN));
		memcpy(&payload_udp[sizeof(headerUDP)+sizeof(adc_samplesIN)], &adc_samplesC1[0], sizeof(adc_samplesC1));
		memcpy(&payload_udp[sizeof(headerUDP)+sizeof(adc_samplesIN)+sizeof(adc_samplesC1)], &adc_samplesC2[0], sizeof(adc_samplesC2));


		udpclient_client_send((char*) &payload_udp, sizeof(payload_udp));

		proc_samples = false;
	}

}


PUTCHAR_PROTOTYPE {
	HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, HAL_MAX_DELAY);
	return ch;
}

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
  if (htim == &htim6)
  {
	  if (!start_tmrs) {
		start_tmrs=true;
	}
//		if (!dac_updt) dac_updt=true;
  }
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*************** END OF FUNCTIONS *********************************************/
