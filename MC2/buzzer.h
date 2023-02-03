/*
 * buzzer.h
 *
 *  Created on: Nov 1, 2022
 *      Author: Dell
 */

#ifndef BUZZER_H_
#define BUZZER_H_

#include "gpio.h"

#define BUZZER_PORT_ID  PORTD_ID
#define BUZZER_PIN_ID   PIN7_ID

void BUZZER_init(void);
void BUZZER_on(void);
void BUZZER_off(void);

#endif /* BUZZER_H_ */
