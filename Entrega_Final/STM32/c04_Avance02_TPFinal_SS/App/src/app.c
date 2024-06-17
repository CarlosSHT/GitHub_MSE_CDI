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
//#include "pid_ctrl.h"
#include "pole_plcmnt.h"
#include "matrix_ops.h"
/******************************************************************************
* Private defines
*******************************************************************************/
#define USE_DAC_BUFFER	1	// Set CubeMX DAC OUT1 Trigger TIMER 8


#define SAMPLES_ADC	40 		// Number samples to send
#define FS_ADC		500
#define DAC_VMAX	2480 	// 4095 = 3.3V, 666 = 2.0V
#define DAC_VMIN	1241	// 4095 = 3.3V, 356 = 1.0V

#define MIN_VOLT	1.0
#define MAX_VOLT	2.0

#define N_SAMPLES 3

#define SAMP_CHNG	50

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

uint16_t adc_dma_buff[3];
uint16_t adc_samplesIN[SAMPLES_ADC];		// Vector with samples to send
uint16_t adc_samplesC1[SAMPLES_ADC];		// Vector with samples to send
uint16_t adc_samplesC2[SAMPLES_ADC];		// Vector with samples to send
uint16_t smp_idx = 0;

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
bool_t cap_adcs = false;


uint8_t i_R = 0;
bool_t flag_VOLT =false;
float R[N_SAMPLES];
float Y_POLEP[2][N_SAMPLES];
float xK[2][1];
float Xi[N_SAMPLES];

pplc_config_t pp_controller;

// Valores experimentales
float ssAz [2][2]={{0.1350, -0.1604},{0.1418, 0.9664}};
float ssBz [2][1]={{0.0177},{0.0037}};
float ssCz [1][2]={{0, 9.0422}};
float valK [1][2]={{4.1231, 3.5563}};
float valK1_a [1][2]={{26.2212,  167.4234}};
//float valK1_b [1][2]={{-1.3937}};
float valK1_b=-1.3937;
float valK0 = 1.3937;



// Valores teoricos
//float ssAz [2][2]={{0.4498, -0.1391},{0.2221, 0.9748}};
//float ssBz [2][1]={{0.0278},{0.0050}};
//float ssCz [1][2]={{0, 5.0080}};
//float valK [1][2]={{3.8039, 2.6439}};
//float valK0 = 1.5279;

float u_n = 0;
float x_i1 = 0;
float_matrix Y_polp;
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
	float_matrix Ad, Bd, Cd, K, K1_a;
	float K0, K1_b;

	Ad.mdir = &ssAz[0][0];
	Ad.rows = 2;
	Ad.cols = 2;

	Bd.mdir = &ssBz[0][0];
	Bd.rows = 2;
	Bd.cols = 1;

	Cd.mdir = &ssCz[0][0];
	Cd.rows = 1;
	Cd.cols = 2;

	K.mdir = &valK[0][0];
	K.rows = 1;
	K.cols = 2;

	K1_a.mdir = &valK1_a[0][0];
	K1_a.rows = 1;
	K1_a.cols = 2;

	K1_b = valK1_b;

	K0 = valK0;

	float PRO[Ad.rows][Bd.cols];
	float_matrix aPro;
	aPro.mdir = &PRO[0][0];
	aPro.rows = 2;
	aPro.cols = 1;

	mult_float_matrix(Ad, Bd, &aPro);


	udpclient_Init(DEST_IP_ADDR0, DEST_IP_ADDR1, DEST_IP_ADDR2, DEST_IP_ADDR3, DEST_PORT);

	HAL_TIM_OnePulse_Start_IT(&htim6, TIM_CHANNEL_ALL);
	HAL_TIM_Base_Start_IT(&htim6);
	smp_idx = 0;

	pole_placement_init(&pp_controller, Ad, Bd, Cd, K, K1_a, K1_b, K0);
}

void app_fsm(void)
{
	MX_LWIP_Process();

	if (start_tmrs) {
		start_tmrs = false;

		HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
//		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, DAC_VMIN);
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, MIN_VOLT*4095/3.3);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buff, sizeof(adc_dma_buff)/sizeof(adc_dma_buff[0]));
		HAL_TIM_Base_Start_IT(&htim8);
	}

	if (cap_adcs) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buff, sizeof(adc_dma_buff)/sizeof(adc_dma_buff[0]));

		adc_samplesIN[smp_idx] = adc_dma_buff[0];
		adc_samplesC1[smp_idx] = adc_dma_buff[1];
		adc_samplesC2[smp_idx] = adc_dma_buff[2];

		smp_idx++;


		R[0] = R[1];
		R[1] = R[2];
		if (!flag_VOLT) {
			R[2] = MIN_VOLT;
		}else {
			R[2] = MAX_VOLT;
		}

		i_R++;
		if (i_R >= 200) {
			i_R = 0;
			flag_VOLT = !flag_VOLT;
		}

		Y_POLEP[0][0] = Y_POLEP[0][1];
		Y_POLEP[1][0] = Y_POLEP[1][1];

		Y_POLEP[0][1] = Y_POLEP[0][2];
		Y_POLEP[1][1] = Y_POLEP[1][2];

		Y_POLEP[0][2] = adc_samplesC1[smp_idx-1]*3.3/4095;
		Y_POLEP[1][2] = adc_samplesC2[smp_idx-1]*3.3/4095;

		xK[0][0] = Y_POLEP[0][1];
		xK[1][0] = Y_POLEP[1][1];

		Y_polp.mdir = &xK[0][0];
		Y_polp.rows = 2;
		Y_polp.cols = 1;

//		u_n = pole_placement_control_I(pp_controller, Y_polp, R[2], Xi[2]);
//		x_i1 = pole_placement_xi_I(pp_controller, Y_polp, R[2], Xi[2]);
//
//		Xi[0] = Xi[1];
//		Xi[1] = Xi[2];
//		Xi[2] = x_i1;


		u_n = pole_placement_control(pp_controller, Y_polp, R[2]);

//		u_n = R[2];
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, u_n*4095/3.3);

		if (smp_idx >= SAMPLES_ADC) {
			smp_idx = 0;
			proc_samples = true;
		}

		cap_adcs = false;
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
//	if (hadc == &hadc1 || hadc == &hadc2) {
//		if (!rdy_midAD) rdy_midAD=true;
//	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == &hadc1) {
		if (!rdy_cpltAD) rdy_cpltAD=true;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim6) {
		if (!start_tmrs) {
			start_tmrs = true;
		}
//		if (!dac_updt) dac_updt=true;
	}

	if (htim == &htim8) {
		if (!cap_adcs) {

			cap_adcs = true;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*************** END OF FUNCTIONS *********************************************/
