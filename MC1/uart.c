 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.c
 *
 * Description: source file for the UART driver
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#include "uart.h"
//#include "gpio.h"
#include <avr/io.h>
#include "std_types.h"
#include "common_macros.h"

/*******************************************************************************
 *                      Functions Definitions	                               *
 *******************************************************************************/
void UART_init (UART_settings * my_uart)
{
	if(my_uart->double_trans_rate ==ON){
		UCSRA = (1<<U2X);              //enabling double transmission mode
	}
	else {}

	UCSRB = (1<<RXEN) | (1<<TXEN); //enabling transmitter and receiver
	if(my_uart->char_size ==nine_bit)
		UCSRB |= (1<<UCSZ2);
	else {}

	UCSRC = (1<<URSEL) ;
	if(my_uart ->char_size ==nine_bit)
		UCSRC |= (1<<UCSZ1) | (1<<UCSZ0);
	else
	{
		UCSRC &= 0xf9;
		UCSRC |= ((my_uart->char_size) << 1);  //inserting according to character size in datasheet
	}

	UCSRC &= ~(1<<UMSEL) | (my_uart->mode << 6);  //inserting mode of uart
	UCSRC &= 0xcf | (my_uart->parity_mode <<4);  //inserting parity mode of uart
	UCSRC &= ~(1<<USBS) | (my_uart->num_stop_bits<<3); //inserting number of stop bits (datasheet)
	UCSRC &= 0xfe | my_uart->clock_polarity ;    //inserting clock polarity

	uint16 ubrr_value;

	if(my_uart->mode ==synch)
	{
		ubrr_value = (uint16) ((F_CPU/(2*my_uart->baud_rate))-1);
	}
	else
	{
		if(bit_is_set(UCSRA,U2X))
		{
			ubrr_value = (uint16) ((F_CPU/(8*my_uart->baud_rate))-1);
		}
		else
		{
			ubrr_value = (uint16) ((F_CPU/(16*my_uart->baud_rate))-1);
		}
	}

UBRRL = ubrr_value;
UBRRH = ubrr_value >>8;
}

void UART_sendByte (uint8 data)
{
	// I will wait till the transmit buffer is empty the I will store data in it
	while (BIT_IS_CLEAR(UCSRA,UDRE));

	UDR = data;
	//the flag is cleared by this step as register is not empty anymore
}

uint8 UART_receiveByte (void)
{
	//I will wait till the uart complete receiving the data then I will
	//return the value in register UDR
	while(BIT_IS_CLEAR(UCSRA,RXC));
	return UDR;
}

void UART_sendString (char * str)
{
	uint8 i=0;
	while (str[i] != '\0')
	{
		UART_sendByte(str[i]);
		i++;
	}
}

void UART_receiveString (char * str)  // passing an empty string and changing it in the function
{
  uint8 i=0;
  str[i]= UART_receiveByte();
  while(str[i] != '#')
  {
	  i++;
	  str[i]= UART_receiveByte();
  }  //after this loop I have the string with '#'

  str[i]='\0';
  //replacing '#' by the null character
}
