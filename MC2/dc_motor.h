/******************************************************************************
 *
 * Module: dc motor
 *
 * File Name: dc_motor.h
 *
 * Description: Header file for the dc motor driver
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include "gpio.h"

typedef enum
{
	STOP, CLOCKWISE ,ANTICLOCKWISE
}Dc_Motor_State;

#define NUM_MOTORS_CONNECTED 1

#define MOTOR1_PORT_ID    PORTB_ID
#define MOTOR1_PIN1_ID    PIN5_ID
#define MOTOR1_PIN2_ID    PIN4_ID

/*h bridge can work on 2 motors , according to
 * number of motors connected whether one or two
 * I will work on it, making it configurable
 */

#if (NUM_MOTORS_CONNECTED==2)
#define MOTOR2_PORT_ID
#define MOTOR2_PIN1_ID
#define MOTOR2_PIN2_ID
#endif

void DCMotor_Init (void);
void DcMotor_Rotate(Dc_Motor_State state,uint8 speed);

#endif /* DC_MOTOR_H_ */
