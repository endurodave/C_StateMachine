#ifndef _MOTOR_H
#define _MOTOR_H

#include "DataTypes.h"
#include "StateMachine.h"

// Motor object structure
typedef struct
{
    INT currentSpeed;
} Motor;

// Event data structure
typedef struct
{
    INT speed;
} MotorData;

// State machine event functions
EVENT_DECLARE(MTR_SetSpeed, MotorData)
EVENT_DECLARE(MTR_Halt, NoEventData)

#endif // _MOTOR_H
