/*
 * RAM.h
 *
 *  Created on: 30/07/2019
 *      Author: A. Pintor
 */

#ifndef RAM_H_
#define RAM_H_


#define FREC_CLK_TIMER (120000000U) // Timer Frequency =  120 MHz
#define BTN_PRESSED IOPORT_LEVEL_LOW
#define BTN_RELEASED IOPORT_LEVEL_HIGH
#define LED_ON IOPORT_LEVEL_LOW
#define LED_OFF IOPORT_LEVEL_HIGH
#define MOTOR_ON IOPORT_LEVEL_HIGH
#define MOTOR_OFF IOPORT_LEVEL_LOW
#define ON IOPORT_LEVEL_HIGH
#define OFF IOPORT_LEVEL_LOW
#define C_FILTER_ORDER  8
#define TS_NUMBER 5


ssp_err_t SError;
uint16_t u16PwmPercent=50;
uint16_t u16Frec_PWM = 1000, u16RPM_Filtered, u16Frec_Sensor_op1 = 0, u16Frec_Sensor_op2 = 0;
uint8_t u8Pulses = 0;
ioport_level_t u1Pin=0;
uint16_t u16RPM_SP = 1000, u16RPM = 0;
uint8_t u8Kp=64, u8Ki=32; // Gains for T=100ms

uint16_t au16Send_DataToLCD[2] = {0};

uint8_t u8Mot_status=0;
uint8_t u8Faults_Counter = 0;
uint8_t U8Ts_Number = 5;

uint32_t u32WDT_Counter ;
wdt_status_t u1WDT_Status;


#endif /* RAM_H_ */
