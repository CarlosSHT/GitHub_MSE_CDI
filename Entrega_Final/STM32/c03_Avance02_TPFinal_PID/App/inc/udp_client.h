/******************************************************************************
* Title                 :   ------
* Filename              :   udp_client.h
* Author                :   Carlos Herrera Trujillo
* Origin Date           :   Jun 8, 2024
* Version               :   x.0.0
* Compiler              :   ------
* Target                :   STM32XXX
* Notes                 :   
*******************************************************************************/


/******************************************************************************
* Define to Prevent Recursive Inclusion
*******************************************************************************/
#ifndef INC_UDP_CLIENT_H_
#define INC_UDP_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* Private Preprocessor Constants
*******************************************************************************/


/******************************************************************************
* Private Includes
*******************************************************************************/
#include <stdint.h>

/******************************************************************************
* Public defines
*******************************************************************************/


/******************************************************************************
* Exported Typedefs
*******************************************************************************/


/******************************************************************************
* Exported constants
*******************************************************************************/

	
/******************************************************************************
* Exported macro
*******************************************************************************/


/******************************************************************************
* Exported functions prototypes
*******************************************************************************/

void udpclient_Init(uint8_t ip0, uint8_t ip1, uint8_t ip2, uint8_t ip3, uint16_t port);

void udpclient_client_send(char *data_msg, uint16_t datasize);


/*******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* INC_UDP_CLIENT_H_ */

/*** End of File **************************************************************/
