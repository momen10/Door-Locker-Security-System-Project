/*
 * buzzer.c
 *
 *  Created on: Nov 1, 2022
 *      Author: Dell
 */

#include "buzzer.h"

void BUZZER_init(void)
{
	GPIO_setupPinDirection(BUZZER_PORT_ID, BUZZER_PIN_ID, PIN_OUTPUT);
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_LOW);
}

void BUZZER_on(void)
{
GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_HIGH);
}
void BUZZER_off(void)
{
	GPIO_writePin(BUZZER_PORT_ID, BUZZER_PIN_ID, LOGIC_LOW);
}
