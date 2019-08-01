/*
 * system.h
 *
 *  Created on: 30/07/2019
 *      Author: jpjmexip
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdbool.h>

void SR_Conf_System(void);
void SR_Motor_Control(void);
void SR_Motor_Status(void);
uint16_t FN_u16PI_Control(int16_t lu16Error);
uint16_t FN_u16Filter(uint16_t lu16Value);
void FN_Enable_Motor(bool lu1Status);
uint16_t FN_u16Read_RPM_SP(void);
void SR_Fault_handle(void);
void SR_Blinking_LED(void);
void SR_Error_handle(void);

#endif /* SYSTEM_H_ */
