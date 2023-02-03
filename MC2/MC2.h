 /******************************************************************************
 *
 * Module: Control ECU
 *
 * File Name: MC1.h
 *
 * Description: header file for the Control ECU
 *
 * Author: Mo'men Ahmed
 *
 *******************************************************************************/
#ifndef MC2_H_
#define MC2_H_

/*******************************************************************************
 *                   CONTROL DEFINITIONS                                   *
 *******************************************************************************/
#define SUCCESS 1
#define FAIL    0
#define PASSKEY_LENGTH            5
#define MATCHED_FIRST_KEYS      0x10
#define UNMATCHED_FIRST_KEYS    0x20
#define MATCHED_SECOND_KEYS     0x15
#define UNMATCHED_SECOND_KEYS   0x25
#define BUZZER_ON               0x50
#define TIMER_START             0x70
#define CHANGE_PASS             0x80
#define PLUS_BYTE               0x11
#define MINUS_BYTE              0x12
#define BUZZER_OFF              0x51
#define TIMER_NOT_START         0x71
#define NO_CHANGE_PASS          0x81
#define UART2_READY             0x13
#define UART1_READY             0x14


#define COMPARE_VALUE_SEC 31250
#endif /* MC2_H_ */
