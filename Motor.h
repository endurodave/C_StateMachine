#ifndef _MOTOR_H
#define _MOTOR_H

#include "StateMachine.h"

// Event data structure
typedef struct
{
    INT speed;
} MotorData;

void MTR_SetSpeed(MotorData* data);
void MTR_Halt(void);

#endif // _MOTOR_H
