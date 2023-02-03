/******************************************************************************
 *
 * Module: dc motor
 *
 * File Name: dc_motor.c
 *
 * Description: source file for dc motor driver
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#include "dc_motor.h"
#include "gpio.h"
#include "pwm.h"

/*this function aims to initiate our motor to
 * the required settings */

void DCMotor_Init (void)
{
  GPIO_setupPinDirection(MOTOR1_PORT_ID, MOTOR1_PIN1_ID, PIN_OUTPUT);
  GPIO_setupPinDirection(MOTOR1_PORT_ID, MOTOR1_PIN2_ID, PIN_OUTPUT);

  GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN1_ID, LOGIC_LOW);
  GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN2_ID, LOGIC_LOW);//turning off the motor initially
}

/*according to the state passed to the function,
 * I will operate the  motor in the given direction
 */

void DcMotor_Rotate(Dc_Motor_State state,uint8 speed)
{
	 PWM_Timer0_Start(speed);
	switch(state)
	{
	case STOP:
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN1_ID, LOGIC_LOW);
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN2_ID, LOGIC_LOW);
			 break;
	case CLOCKWISE:
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN1_ID, LOGIC_HIGH);
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN2_ID, LOGIC_LOW);
			 break;
	case ANTICLOCKWISE:
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN1_ID, LOGIC_LOW);
			 GPIO_writePin(MOTOR1_PORT_ID, MOTOR1_PIN2_ID, LOGIC_HIGH);
			 break;
	}
}

