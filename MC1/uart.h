 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: header file for the UART driver
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#ifndef UART_H_
#define UART_H_
#include "std_types.h"

/*******************************************************************************
 *                     enums                                *
 *******************************************************************************/
typedef enum
{
  baud_2400 =2400 , baud_4800 =4800, baud_9600=9600
}UART_baudRate;

typedef enum {
	OFF ,ON
}UART_doubleTransRate;

typedef enum{
	five_bit , six_bit , seven_bit , eight_bit , nine_bit=7
}UART_characterSize;

typedef enum {
	asynch ,synch
}UART_mode;

typedef enum{
	DISABLE, even_parity=2 , odd_parity
}UART_parityMode;

typedef enum{
	one_bit , two_bits
}UART_stopBits;

typedef enum {
	rising_falling , falling_rising
}UART_clockPolarity;

typedef struct{
	UART_baudRate baud_rate;
	UART_doubleTransRate double_trans_rate ;
	UART_characterSize  char_size;
	UART_mode   mode;
	UART_parityMode parity_mode ;
	UART_stopBits num_stop_bits;
	UART_clockPolarity clock_polarity;
}UART_settings;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/
void UART_init (UART_settings * my_uart);
void UART_sendByte (uint8 data);
uint8 UART_receiveByte (void);
void UART_sendString (char * str);
void UART_receiveString (char * str);
#endif /* UART_H_ */
