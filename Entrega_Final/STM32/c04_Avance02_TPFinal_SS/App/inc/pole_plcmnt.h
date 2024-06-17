/******************************************************************************
* Title                 :   ------
* Filename              :   pole_plcmnt.h
* Author                :   Carlos Herrera Trujillo
* Origin Date           :   Jun 14, 2024
* Version               :   x.0.0
* Compiler              :   ------
* Target                :   STM32XXX
* Notes                 :   
*******************************************************************************/


/******************************************************************************
* Define to Prevent Recursive Inclusion
*******************************************************************************/
#ifndef INC_POLE_PLCMNT_H_
#define INC_POLE_PLCMNT_H_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* Private Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Private Includes
*******************************************************************************/
#include "matrix_ops.h"
#include <stdint.h>

/******************************************************************************
* Public defines
*******************************************************************************/


/******************************************************************************
* Exported Typedefs
*******************************************************************************/


typedef struct {
	float_matrix Ad;
	float_matrix Bd;
	float_matrix Cd;
	float_matrix K;
	float_matrix K1_a;
	float K1_b;
	float Ko;
} pplc_config_t;

/******************************************************************************
* Exported constants
*******************************************************************************/

	
/******************************************************************************
* Exported macro
*******************************************************************************/


/******************************************************************************
* Exported functions prototypes
*******************************************************************************/
void pole_placement_init(pplc_config_t *config, float_matrix Az, float_matrix Bz, float_matrix Cz, float_matrix K, float_matrix K1_a, float K1_b, float K0);
float pole_placement_control(pplc_config_t config, float_matrix x_lcg, float reference);

float pole_placement_xi_I(pplc_config_t config, float_matrix x_lci, float r_i, float x_i);
float pole_placement_control_I(pplc_config_t config, float_matrix x_lcg, float r_i, float x_i);



/*******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_POLE_PLCMNT_H_ */

/*** End of File **************************************************************/
