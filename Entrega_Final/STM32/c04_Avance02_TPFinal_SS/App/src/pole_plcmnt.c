/*******************************************************************************
* Title                 :   ------
* Filename              :   pole_plcmnt.c
* Author                :   Carlos Herrera Trujillo
* Origin Date           :   Jun 14, 2024
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
#include "pole_plcmnt.h"

/******************************************************************************
* Private defines
*******************************************************************************/


/******************************************************************************
* Private Typedefs
*******************************************************************************/




/******************************************************************************
* Private Macros
*******************************************************************************/


/******************************************************************************
* Public Variables
*******************************************************************************/


/******************************************************************************
* Private Variables
*******************************************************************************/


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

void pole_placement_init(pplc_config_t *config, float_matrix Az, float_matrix Bz, float_matrix Cz, float_matrix K, float_matrix K1_a, float K1_b, float K0)
{
	config->Ad.mdir = Az.mdir;
	config->Ad.rows = Az.rows;
	config->Ad.cols = Az.cols;

	config->Bd.mdir = Bz.mdir;
	config->Bd.rows = Bz.rows;
	config->Bd.cols = Bz.cols;

	config->Cd.mdir = Cz.mdir;
	config->Cd.rows = Cz.rows;
	config->Cd.cols = Cz.cols;

	config->K.mdir = K.mdir;
	config->K.rows = K.rows;
	config->K.cols = K.cols;


	config->K1_a.mdir = K1_a.mdir;
	config->K1_a.rows = K1_a.rows;
	config->K1_a.cols = K1_a.cols;

	config->K1_b = K1_b;

	config->Ko = K0;
}

float pole_placement_control(pplc_config_t config, float_matrix x_lcg, float reference)
{
	float aux_p[config.K.rows][x_lcg.cols];

	float_matrix K_xlcg;
	K_xlcg.mdir = &aux_p[0][0];
	K_xlcg.rows = 1;
	K_xlcg.cols = 1;

	mult_float_matrix(config.K, x_lcg, &K_xlcg);

	return (config.Ko * reference) - *(K_xlcg.mdir);
//	return (config->Ko * reference) - (config->K * state);
}

float pole_placement_control_I(pplc_config_t config, float_matrix x_lcg, float r_i, float x_i)
{
	float aux_p[config.K1_a.rows][x_lcg.cols];

	float_matrix K_xlcg;
	K_xlcg.mdir = &aux_p[0][0];
	K_xlcg.rows = 1;
	K_xlcg.cols = 1;

	mult_float_matrix(config.K1_a, x_lcg, &K_xlcg);

	return r_i - *(K_xlcg.mdir) - config.K1_b * x_i;
}


float pole_placement_xi_I(pplc_config_t config, float_matrix x_lci, float r_i, float x_i)
{
	float aux_p[config.Cd.rows][x_lci.cols];

	float_matrix C_xlci;
	C_xlci.mdir = &aux_p[0][0];
	C_xlci.rows = 1;
	C_xlci.cols = 1;

	mult_float_matrix(config.Cd, x_lci, &C_xlci);

	return x_i + r_i - *(C_xlci.mdir);
//	return r_i - *(K_xlcg.mdir) - config.K1_b * x_i;
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*************** END OF FUNCTIONS *********************************************/
