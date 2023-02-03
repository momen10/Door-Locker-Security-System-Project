 /******************************************************************************
 *
 * Module: TWI(I2C)
 *
 * File Name: twi.h
 *
 * Description: Header file for the TWI(I2C) AVR driver
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/ 

#ifndef TWI_H_
#define TWI_H_

#include "std_types.h"

/*******************************************************************************
 *                      Preprocessor Macros                                    *
 *******************************************************************************/

/* I2C Status Bits in the TWSR Register */
#define TWI_START         0x08 /* start has been sent */
#define TWI_REP_START     0x10 /* repeated start */
#define TWI_MT_SLA_W_ACK  0x18 /* Master transmit ( slave address + Write request ) to slave + ACK received from slave. */
#define TWI_MT_SLA_R_ACK  0x40 /* Master transmit ( slave address + Read request ) to slave + ACK received from slave. */
#define TWI_MT_DATA_ACK   0x28 /* Master transmit data and ACK has been received from Slave. */
#define TWI_MR_DATA_ACK   0x50 /* Master received data and send ACK to slave. */
#define TWI_MR_DATA_NACK  0x58 /* Master received data but doesn't send ACK to slave. */

#define NORMAL_MODE_FREQ     100000ul
#define FAST_MODE_FREQ       400000ul
#define FAST_MODE_PLUS_FREQ  1000000ul
#define HIGH_SPEED_MODE_FREQ 3400000ul

/*******************************************************************************
 *                     enums                                *
 *******************************************************************************/
typedef enum{
	NORMAL_MODE_TWI , FAST_MODE  , FAST_MODE_PLUS , HIGH_SPEED_MODE
}TWI_dataRate;

typedef enum{
	ZERO, ONE, TWO, THREE
}TWI_prescaler;

typedef uint8 TWI_address;

typedef struct{
	TWI_dataRate data_rate;
	TWI_address  address;
}TWI_config;
/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void TWI_init(TWI_config * twi);
void TWI_start(void);
void TWI_stop(void);
void TWI_writeByte(uint8 data);
uint8 TWI_readByteWithACK(void);
uint8 TWI_readByteWithNACK(void);
uint8 TWI_getStatus(void);


#endif /* TWI_H_ */
