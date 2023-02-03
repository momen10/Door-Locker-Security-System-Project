 /******************************************************************************
 *
 * Module: Human Machine Interface ECU
 *
 * File Name: MC1.c
 *
 * Description: source file for the human machine interface controller
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#include "MC1.h"
#include "keypad.h"
#include "lcd.h"
#include "std_types.h"
#include <util/delay.h>
#include "uart.h"
#include "Timer1.h"
#include "avr/io.h"

/*******************************************************************************
 *                   HMI FUNCTION PROTOTYPES                                   *
 *******************************************************************************/
void HMI_init(void);
void get_first_passkey();
uint8 check_matched_first_keys(void);
uint8 check_matched_second_keys(void);
uint8 check_uart2_byte (void);
void get_new_passkey(void);
void HMI_ticks (void);
void lcd_open_door(void);
void lcd_buzzer_on (void);
/*******************************************************************************
 *                   HMI GLOBAL VARIABLES                                   *
 *******************************************************************************/
uint8 key=0;
uint8 wrong_pass_count=0;
uint8 hmi_second_ticks=0;
/*******************************************************************************
 *                  HMI MAIN FUNCTION                                  *
 *******************************************************************************/
int main(void)
{
	HMI_init();
	SREG |= (1<<7);  //ENABLING THE GLOBAL INTERRUPT BIT

	while(1)
	{
		UART_sendByte(UART1_READY);   //SEND TO CONTROL_ECU THAT HMI IS READY
		while(!check_uart2_byte());   //WAIT UNTIL CONTROL_ECU REPLIES WITH READY
		get_first_passkey();          /*GET FROM USER FIRST AND SECOND PASSKEYS AND SEND THEM TO CONTROL_ECU*/
        _delay_ms(500);
        get_first_passkey();
        _delay_ms(500);

        uint8 check_first_entry= check_matched_first_keys(); //COMPARE THEM
        if(check_first_entry==SUCCESS)
        {
        	LCD_clearScreen();
        	LCD_displayString("true");
        	_delay_ms(1000);
        }
        else if(check_first_entry==FAIL)
        {
        	LCD_displayString("wrong");
        	_delay_ms(1000);
        	continue;
        }

/*ENTERING A WHILE LOOP ONCE A PASSKEY HAS BEEN SAVED IN EEPROM
 * DISPLAYING THE OPTIONS OF THE DOOR AND GETTING INPUTS
FROM USER */
		while(1)
		{
			uint8 check_second_entry=0;
			key=0;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0, 0,"+ : Open Door");
			LCD_displayStringRowColumn(1, 0, "- : Change Pass");
			while(key!= '+' && key!= '-')   //WAIT UNTIL THE USER ENTER A '+' OR '-'
			{
				key=KEYPAD_getPressedKey();
				_delay_ms(500);
			}

			if(key=='+')         //OPEN DOOR OPTION
			{
				UART_sendByte(PLUS_BYTE);
				while(check_second_entry==FAIL)  //CONTINUE GETTING PASSKEY FROM USER AS LONG
				{                                //AS HE ENTERS A WRONG PASSKEY
					get_new_passkey();
					_delay_ms(100);
					check_second_entry = check_matched_second_keys(); //SENDING THE NEW PASSKEY TO
                                                      //CONTROL ECU AND WAITING FOR THE RESPONCE

					if(check_second_entry== SUCCESS) //IF PASSKEY IS CORRECT
					{                               //SEND TO CONTROL ECU A COMMAND TO
						wrong_pass_count=0;          //OPEN THE DOOR USING THE TIMER
						UART_sendByte(TIMER_START);
						UART_sendByte(BUZZER_OFF);
						lcd_open_door();
						break;
					}
					else if(check_second_entry==FAIL) //IF PASSKEY IS WRONG, ASK HIM
					{                                 //TO ENTER A NEW PASSKEY,
						UART_sendByte(TIMER_NOT_START);//IF HE ENTERS 3 WRONG PASSKEYS
						wrong_pass_count++;            //IN A ROW, SEND TO THE CONTROL
						if(wrong_pass_count==3)        //ECU TO A COMMAND TO ENABLE THE BUZZER
						{
							wrong_pass_count=0;
							UART_sendByte(BUZZER_ON);
							lcd_buzzer_on();
							break;
						}
						else
						{
							UART_sendByte(BUZZER_OFF);
							LCD_clearScreen();
							LCD_displayString("wrong password");
							LCD_displayStringRowColumn(1, 0, "try again");
							_delay_ms(1000);
							continue;
						}
					}
				}
			}
            //CHANGE PASSKEY OPTION
			else if(key=='-')
			{
				UART_sendByte(MINUS_BYTE);      //CONTINUE GETTING PASSKEY FROM USER AS LONG
				while(check_second_entry==0)     //AS HE ENTERS A WRONG PASSKEY
				{
					get_new_passkey();
					_delay_ms(100);
					check_second_entry = check_matched_second_keys();

					if(check_second_entry==SUCCESS) //IF HE ENTERS A CORRECT PASSKEY
					{                           //SEND TO THE CONTROL ECU COMMAND TO CHANGE PASSKEY
						LCD_clearScreen();      //AND RETURN TO THE ENTRY OF PROGRAM
						LCD_displayString("success");
						wrong_pass_count=0;
						UART_sendByte(CHANGE_PASS);
						_delay_ms(100);
						UART_sendByte(BUZZER_OFF);
						break;
					}

					else if(check_second_entry== FAIL) //IF PASSKEY IS WRONG, ASK HIM
					{                                  //TO ENTER A NEW PASSKEY,
						wrong_pass_count++;            //IF HE ENTERS 3 WRONG PASSKEYS
						UART_sendByte(NO_CHANGE_PASS); //IN A ROW, SEND TO THE CONTROL
						if(wrong_pass_count==3)        //ECU TO A COMMAND TO ENABLE THE BUZZER
						{
							wrong_pass_count=0;
							UART_sendByte(BUZZER_ON);
							lcd_buzzer_on();
						}
						else
						{
							UART_sendByte(BUZZER_OFF);
							LCD_clearScreen();
							LCD_displayString("wrong password ");
							LCD_intgerToString(wrong_pass_count);
							LCD_displayStringRowColumn(1, 0, "try again");
							_delay_ms(1000);
							continue;
						}
					}
				}
				break;
			}
		}
	}
}

/*THIS FUNCTION AIMS TO INITIALIZE ALL MODULES IN HMI CONTROLLER
 * INCLUDING LCD, UART, TIMER1 */
void HMI_init(void)
{
	LCD_init();
	UART_settings my_uart= {baud_9600, ON, eight_bit , asynch, DISABLE, one_bit, rising_falling};
	UART_init(&my_uart);

	Timer1_Config my_timer1 ={0, COMPARE_VALUE_SEC, COMPARE_MODE,\
	    		F_CPU_256, DISCONNECT_OC1A, DISCONNECT_OC1B};
	Timer1_SetCallBack(HMI_ticks);
	Timer1_init(&my_timer1);
}

uint8 try=0;
/*THIS FUNCTION IS RESPONSIBLE FOR GETTING FROM USER
 * FIRST AND SECOND PASSKEYS IN THE BEGINNING OF SYSTEM AND SEND THEM TO THE
 CONTROL ECU BY UART TO COMPARE THEM*/
void get_first_passkey()
{
	uint8 pass[7]={0};
	try++;
	if(try==1)
	{
		LCD_clearScreen();
		LCD_displayString("plz enter pass :");
		LCD_moveCursor(1, 0);
	}
	else if(try==2)         //GETTING SECOND PASSKEY
	{
		try=0;
		LCD_clearScreen();
		LCD_displayString("plz re-enter the");
		LCD_displayStringRowColumn(1, 0, "same pass:");
		LCD_moveCursor(1, 10);
	}

	uint8 i;
	for(i=0; i<PASSKEY_LENGTH;i++)
	{
		key=KEYPAD_getPressedKey();
		_delay_ms(500);
		if(key>= 0 && key<= 9)   //GETTING A PASSKEY AS NUMBERS ONLY, NO SYMBOLS
		{
			pass[i]=key;
			LCD_displayCharacter('*');
		}
		else
		{
			i--;
		}
	}
	while(KEYPAD_getPressedKey()!= 13);
	pass[5]='#';                 //MODULATING THE STRING ENTERED BY USER TO BE ABLE
	pass[6]='\0';                //TO SEND IT BU UART TO THE CONTROL ECU
	UART_sendString(pass);
_delay_ms(100);
}

/*THIS FUNCTION AIMS TO GET RESPONCE COMMAND FROM THE CONTROL ECU EITHER
 * THE FIRST 2 PASSKEYS ARE MATCHED OR NOT */
uint8 check_matched_first_keys(void)
{
	uint8 received_byte=0;
	while(received_byte!=MATCHED_FIRST_KEYS && received_byte!=UNMATCHED_FIRST_KEYS)
	{
		received_byte=UART_receiveByte();
	}
	if(received_byte==MATCHED_FIRST_KEYS)
	{
		return SUCCESS;
	}
	else if(received_byte ==UNMATCHED_FIRST_KEYS)
	{
		return FAIL;
	}
}

/*THIS FUNCTION AIMS TO GET RESPONCE COMMAND FROM THE CONTROL ECU EITHER
 * THE INPUT PASSKEY IS MATCHED WITH THE PASSKEY SAVED IN EEPROM OR NOT */
uint8 check_matched_second_keys(void)
{
	uint8 received_byte=0;
	while(received_byte!=MATCHED_SECOND_KEYS && received_byte!=UNMATCHED_SECOND_KEYS)
	{
		received_byte=UART_receiveByte();
	}
	if(received_byte==MATCHED_SECOND_KEYS)
	{
		return SUCCESS;
	}
	else if(received_byte ==UNMATCHED_SECOND_KEYS)
	{
		return FAIL;
	}
}

/*THIS FUNCTION AIMS TO GET FROM USER NEW PASSKEY
 * AFTER DISPLAYING OPTIONS MENU */
void get_new_passkey(void)
{
	key=0xff;
	uint8 new_passkey[7]={0};
	LCD_clearScreen();
	LCD_displayString("plz enter pass :");
	LCD_moveCursor(1, 0);

	uint8 i;
	for(i=0; i<PASSKEY_LENGTH;i++)
	{
		key=KEYPAD_getPressedKey();
		_delay_ms(500);
		if(key>= 0 && key<= 9)
		{
			new_passkey[i]=key;
			LCD_displayCharacter('*');
		}
		else
		{
			i--;
		}
	}

	while(KEYPAD_getPressedKey()!= 13);
	new_passkey[i]='#';
	new_passkey[i+1]='\0';
	UART_sendString(new_passkey);
	_delay_ms(100);
}

// THIS IS THE CALLBACK FUNCTION OF HMI, JUST INCREMENTING EACH ONE CLOCK CYCLE
//OF THE TIMER
void HMI_ticks (void)
{
	hmi_second_ticks++;
}

//THIS FUNCTION AIMS TO DISPLAY ON LCD "OPENING DOOR" WHILE THE DOOR IS OPENING
void lcd_open_door(void)
{
	hmi_second_ticks=0;
	LCD_clearScreen();
	while(hmi_second_ticks<OPENING_DOOR_TIME)
	{
		LCD_displayStringRowColumn(0, 0, "Opening Door");
	}
}

//THIS FUNCTION IS ENABLED WHEN THE USER ENTERED PASSKEY 3 TIMES IN A ROW
//IT WILL DISPLAY "ERROR" FOR ONE MINUTE WHILE THE BUZZER IS WORKING
void lcd_buzzer_on (void)
{
	hmi_second_ticks=0;
	LCD_clearScreen();
	while(hmi_second_ticks<ONE_MINUTE)
	{
		LCD_displayStringRowColumn(0, 0, "ERROR");
	}
}

//THIS FUNCTION IS WAITING THE CONTROL ECU TO SEND IT A READY BYTE TO RETURN
uint8 check_uart2_byte (void)
{
  uint8 uart_byte =0;
  while(uart_byte != UART2_READY)
  {
	  uart_byte= UART_receiveByte();
  }
  if(uart_byte==UART2_READY)
  {
	  return SUCCESS;
  }
  else
	  return FAIL;
}
