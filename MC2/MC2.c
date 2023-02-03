 /******************************************************************************
 *
 * Module: Control ECU
 *
 * File Name: MC2.c
 *
 * Description: source file for the Control ECU
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#include "MC2.h"
#include "uart.h"
#include "util/delay.h"
#include "external_eeprom.h"
#include "twi.h"
#include "gpio.h"
#include "buzzer.h"
#include "dc_motor.h"
#include "Timer1.h"
#include <avr/interrupt.h>
#include "pwm.h"

/*******************************************************************************
 *                   CONTROL_ECU FUNCTION PROTOTYPES                                   *
 *******************************************************************************/
void CONTROL_init(void);
void receive_first_passkey (uint8 *);
uint8 check_passkeys(uint8 *, uint8 *);
uint8 check_new_passkey(uint8 *);
uint8 check_buzzer_byte(void);
uint8 check_change_pass_byte (void);
void open_door(void);
void increment_ticks(void);
uint8 check_timer_byte(void);
void buzzer_work(void);
void receive_new_pass(uint8 *);
void save_passkey(uint8 *);
void get_eeprom_reading();
uint8 check_p_m_byte(void);
uint8 check_uart1_byte (void);

/*******************************************************************************
 *                   CONTROL_ECU GLOBAL VARIABLES                              *
 *******************************************************************************/
uint8 correct_passkey[7];
uint8 new_pass[7];
uint8 eeprom_reading[5];
uint8 second_ticks=0;
uint8 wrong=0;

int main(void)
{
	CONTROL_init();
    SREG |= (1<<7);      //ENABLING THE GLOBAL INTERRUPT BIT

	while(1)
	{
		UART_sendByte(UART2_READY);  //SEND TO HMI THAT CONTROL_ECU IS READY
		while(!check_uart1_byte());  //WAIT UNTIL HMI REPLIES WITH READY
		uint8 received_pass1[7]={0} ;
		uint8 received_pass2[7]={0};
       receive_first_passkey(received_pass1);  //RECEIVING THE FIRST 2 PASSKEYS
       _delay_ms(100);
       receive_first_passkey(received_pass2);
       _delay_ms(100);

		if(check_passkeys(received_pass1,received_pass2)) //IF THE ARE MATCHED
		{                                                //SAVE THE PASSKEY IN THE EEPROM
			UART_sendByte(MATCHED_FIRST_KEYS);
            save_passkey(received_pass1);
            get_eeprom_reading();
            _delay_ms(500);
		}
		else
		{
			UART_sendByte(UNMATCHED_FIRST_KEYS);
			continue;
		}

		while(1)
		{
			uint8 check_input_pass=FAIL ;
			LCD_clearScreen();
			uint8 p_m_byte=0;
			p_m_byte=check_p_m_byte();     //WAITING USER TO ENTER '+' OR '-'
			switch(p_m_byte)
			{
			case PLUS_BYTE: //OPEN DOOR OPTION
				while(check_input_pass==FAIL)    //GETTING NEW INPUT PASSKEYS
				{                                // AS LONG AS HE ENTERS WRONG PASSKEY
				receive_new_pass(new_pass);
				check_input_pass = check_new_passkey(new_pass);
				if(check_input_pass==SUCCESS)
				{
                  UART_sendByte(MATCHED_SECOND_KEYS);
                  _delay_ms(100);
                  if(check_timer_byte()==SUCCESS)  //IF HMI SEND TO CONTROL A COMMAND
                  {                                //TO OPEN THE DOOR
                	  open_door();                 //THE CONTROL ECU OPENS THE DOOR
                  }
                  else{

                  }
                  if(check_buzzer_byte()==SUCCESS) //IF HMI SEND TO CONTROL A COMMAND
                  {                                //TO OPEN THE BUZZER
                	 buzzer_work();                //THE CONTROL ECU TURN ON THE BUZZER
                  }
                  else
                  {

                  }
                  break;
				}
				else if(check_input_pass ==FAIL)
				{
					UART_sendByte(UNMATCHED_SECOND_KEYS);
					_delay_ms(100);
					if(check_timer_byte()==SUCCESS)
					{
						open_door();
					}
					else{

					}
					if(check_buzzer_byte()==SUCCESS)   //IN CASE USER ENTERS WRONG PASSKEY
					{                                  //3 TIMES
						buzzer_work();
					}
					else
					{

					}
				}
				continue;
				}
				break;

			case MINUS_BYTE:  //CHANGE PASSKEY OPTION
				while(check_input_pass==FAIL)
				{
					receive_new_pass(new_pass);
					check_input_pass = check_new_passkey(new_pass);

					if(check_input_pass==SUCCESS)
					{
						UART_sendByte(MATCHED_SECOND_KEYS);
						_delay_ms(100);
						uint8 change_pass_byte=check_change_pass_byte();
						_delay_ms(100);
						uint8 buzzer_byte = check_buzzer_byte();
						_delay_ms(100);
						if(change_pass_byte==SUCCESS)
						{

						}
						else if(check_change_pass_byte()==FAIL)
						{

						}
						if(buzzer_byte==SUCCESS)
						{
							buzzer_work();
						}
						else if(buzzer_byte==FAIL)
						{

						}
						break;   //BREAKING FROM THIS LOOP TO GO THE OUTER LOOP
						         //AS BEGINNING OF THE SYSTEM
					}
					else if(check_input_pass==FAIL)
					{
						UART_sendByte(UNMATCHED_SECOND_KEYS);
						_delay_ms(100);
						uint8 change_pass_byte=check_change_pass_byte();
						_delay_ms(100);
                        uint8 buzzer_byte = check_buzzer_byte();
						_delay_ms(100);
						if(change_pass_byte==SUCCESS)
						{
							break;
						}
						else if(change_pass_byte==FAIL)
						{

						}
						if(buzzer_byte==SUCCESS)
						{
							buzzer_work();
						}
						else if(buzzer_byte==FAIL)
						{

						}
					}
				}
			 }
		break;
		}
	}
}

/*THIS FUNCTION AIMS TO INITIALIZE ALL MODULSs IN CONTROL ECU
 * INCLUDING LCD, UART, TIMER1 */
void CONTROL_init(void)
{
    LCD_init();
    BUZZER_init();
    UART_settings my_uart= {baud_9600, ON, eight_bit , asynch, DISABLE, one_bit, rising_falling};
    UART_init(&my_uart);

    GPIO_setupPinDirection(PORTC_ID, PIN0_ID,PIN_OUTPUT);
    GPIO_setupPinDirection(PORTC_ID, PIN1_ID,PIN_OUTPUT);
    TWI_config my_TWI = {FAST_MODE, 0b00000010};
    /* Initialize the TWI/I2C Driver */
    TWI_init(&my_TWI);
    DCMotor_Init();

    Timer1_Config my_timer1 ={0, COMPARE_VALUE_SEC, COMPARE_MODE,\
    		F_CPU_256, DISCONNECT_OC1A, DISCONNECT_OC1B};
    Timer1_SetCallBack(increment_ticks);
	Timer1_init(&my_timer1);
}

//FUNCTION TO RECEIVE FIRST AND SECOND PASSKEYS BY UART
void receive_first_passkey (uint8 * pass)
{
	UART_receiveString(pass);
}

//THIS FUNCTION RESPONSIBLE FOR COMPARING THE INPUT PASSKEY
//WITH THE PASSKEY SAVED IN THE EEPROM
uint8 check_new_passkey(uint8 * pass)
{
	get_eeprom_reading();
	for (uint8 i=0; i<5; i++)
	{
		if (pass[i]!= correct_passkey[i])
		{
			return FAIL;
		}
	}
	return SUCCESS;
}

//THIS FUNCTION IS RESPONSIBLE FOR SAVING THE PASSKEY IN THE EXTERNAL EEPROM
//TO BE COMPARED WITH IN THE NEXT TRIALS
void save_passkey(uint8 * pass)
{
	uint16 address=0;
	uint8 i=0;
	for(uint8 j=0;j<5; j++)
	{
		correct_passkey[j]= pass[j];
	}
	for(; i<5;i++, address+=8)
	{
		EEPROM_writeByte(address, pass[i]);
		_delay_ms(20);
	}

}

//THIS FUNCTION IS WAITING FROM THE HMI A COMMAND EITHER TO ENABLE THE
//BUZZER OR NOT
//NOTE: BUZZER IS NOT WORKING IN PROTEUS, SO I USED A RED LED INSTEAD
uint8 check_buzzer_byte(void)
{
	uint8 buzzer_byte=0;
	while(buzzer_byte != BUZZER_ON && buzzer_byte != BUZZER_OFF)
	{
		buzzer_byte = UART_receiveByte();
	}
	if(buzzer_byte==BUZZER_ON)
		return SUCCESS;
	else if(buzzer_byte== BUZZER_OFF)
		return FAIL;
}

//THIS FUNCTION AIMS TO OPEN THE DOOR FOR A PERIOD OF TIME USING THE TIMER
void open_door(void)
{
	second_ticks=0;
	do
	{
		DcMotor_Rotate(CLOCKWISE, 100);
      GPIO_writePin(PORTB_ID, PIN4_ID, 1);
      GPIO_writePin(PORTB_ID, PIN5_ID, 0);
	}
	while(second_ticks<15);

	do
	{
		DcMotor_Rotate(STOP, 0);
	  GPIO_writePin(PORTB_ID, PIN4_ID, 0);
	  GPIO_writePin(PORTB_ID, PIN5_ID, 0);
		}
	while(second_ticks<18);
	do
	{
		DcMotor_Rotate(ANTICLOCKWISE, 100);
	  GPIO_writePin(PORTB_ID, PIN4_ID, 0);
	  GPIO_writePin(PORTB_ID, PIN5_ID, 1);
		}
	while(second_ticks<33);
   DcMotor_Rotate(STOP, 0);
}

// THIS IS THE CALLBACK FUNCTION OF CONTROL_ECU, JUST INCREMENTING
//EACH ONE CLOCK CYCLE OF THE TIMER
void increment_ticks(void)
{
	second_ticks ++;
}

//THIS FUNCTION IS WAITING FROM THE HMI A COMMAND EITHER TO ENABLE THE
//TIMER OR NOT
uint8 check_timer_byte(void)
{
	uint8 timer_byte=0;
	while(timer_byte != TIMER_START && timer_byte != TIMER_NOT_START)
	{
		timer_byte = UART_receiveByte();
	}
	if(timer_byte == TIMER_START)
		return SUCCESS;
	else if(timer_byte ==TIMER_NOT_START)
		return FAIL;
}

//THIS FUNCTION IS WAITING FROM THE HMI A COMMAND WHETHER
//TO CHANGE THE PASSKEY OR NOT
uint8 check_change_pass_byte (void)
{
	uint8 change_byte =0;
	change_byte = UART_receiveByte();
	if(change_byte== CHANGE_PASS)
		return SUCCESS;
	else
		return FAIL;
}

//THIS FUNCTION AIMS TO ENABLING THE BUZZER FOR 1 MINUTE USING THE TIMER
void buzzer_work(void)
{
	second_ticks=0;
	while(second_ticks<60)
	{
		BUZZER_on();
	}
	BUZZER_off();
}

//A FUNCTION JUST TO RECEIVE ANY PASSKEY FROM HMI BY UART
void receive_new_pass(uint8 * new_pass)
{
	UART_receiveString(new_pass);
}

//THIS FUNCTION IS RESPONSIBLE FOR CHECKING THE FIRST INPUT
// 2 PASSKEYS WHETHER THEY ARE MATCHED OR NOT
uint8 check_passkeys(uint8 *pass1, uint8 * pass2)
{
	for (uint8 i=0; i<5; i++)
	{
		if(pass1[i] != pass2[i])
		{
		 return FAIL;
		}
	}
	return SUCCESS;
}

//THIS FUNCTION GETS THE DATA IN THE FIRST 5 BYTES IN THE
//EEPROM TO KNOW THE SAVED PASSKEY AT THE MOMENT
void get_eeprom_reading()
{
	uint16 address=0;
	for(uint8 i=0; i<5;i++, address+=8)
	{
		EEPROM_readByte(address, &eeprom_reading[i]);
		_delay_ms(20);
	}
}

//THIS FUNCTION IS WAITING FROM THE HMI A COMMAND WHETHER
//A '+' OR '-' HAS BEEN ENTERED
uint8 check_p_m_byte(void)
{
	uint8 p_m_byte=0;
	while(p_m_byte != PLUS_BYTE && p_m_byte != MINUS_BYTE)
	{
		p_m_byte = UART_receiveByte();
	}
	return p_m_byte;
}

//THIS FUNCTION IS WAITING FROM THE HMI A COMMAND WHETHER
//HMI IS READY OR NOT USING UART
uint8 check_uart1_byte (void)
{
  uint8 uart_byte =0;
  while(uart_byte != UART1_READY)
  {
	  uart_byte= UART_receiveByte();
  }
  if(uart_byte==UART1_READY)
  {
	  return SUCCESS;
  }
  else
	  return FAIL;
}
