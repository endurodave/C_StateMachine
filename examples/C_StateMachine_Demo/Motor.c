// Motor state machine: Idle -> Start -> ChangeSpeed -> Stop -> Idle
// The same state machine as Motor.c in the repository root, logging with
// DemoLog() instead of printf so it shows on an Arduino serial port.

#include "Motor.h"
#include "StateMachine.h"
#include <stdio.h>

// State enumeration order must match the order of state
// method entries in the state map
enum States
{
    ST_IDLE,
    ST_STOP,
    ST_START,
    ST_CHANGE_SPEED,
    ST_MAX_STATES
};

// State machine state functions
STATE_DECLARE(Idle, NoEventData)
STATE_DECLARE(Stop, NoEventData)
STATE_DECLARE(Start, MotorData)
STATE_DECLARE(ChangeSpeed, MotorData)

// State map to define state function order
BEGIN_STATE_MAP(Motor)
    STATE_MAP_ENTRY(ST_Idle)
    STATE_MAP_ENTRY(ST_Stop)
    STATE_MAP_ENTRY(ST_Start)
    STATE_MAP_ENTRY(ST_ChangeSpeed)
END_STATE_MAP(Motor)

static void LogState(SM_StateMachine* self, const char* state, INT speed)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%s %s: %d", self->name, state, (int)speed);
    DemoLog(buf);
}

// Set motor speed external event
EVENT_DEFINE(MTR_SetSpeed, MotorData)
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(ST_START)          // ST_Idle
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_Stop
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)   // ST_Start
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)   // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, pEventData)
}

// Halt motor external event
EVENT_DEFINE(MTR_Halt, NoEventData)
{
    BEGIN_TRANSITION_MAP                        // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)     // ST_Idle
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)     // ST_Stop
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_Start
        TRANSITION_MAP_ENTRY(ST_STOP)           // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, pEventData)
}

// State machine sits here when motor is not running
STATE_DEFINE(Idle, NoEventData)
{
    Motor* pInstance = SM_GetInstance(Motor);
    LogState(self, "ST_Idle", pInstance->currentSpeed);
}

// Stop the motor
STATE_DEFINE(Stop, NoEventData)
{
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance->currentSpeed = 0;
    LogState(self, "ST_Stop", pInstance->currentSpeed);

    // Transition to ST_Idle via an internal event
    SM_InternalEvent(ST_IDLE, NULL);
}

// Start the motor going
STATE_DEFINE(Start, MotorData)
{
    ASSERT_TRUE(pEventData);
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance->currentSpeed = pEventData->speed;
    LogState(self, "ST_Start", pInstance->currentSpeed);
}

// Changes the motor speed once the motor is moving
STATE_DEFINE(ChangeSpeed, MotorData)
{
    ASSERT_TRUE(pEventData);
    Motor* pInstance = SM_GetInstance(Motor);
    pInstance->currentSpeed = pEventData->speed;
    LogState(self, "ST_ChangeSpeed", pInstance->currentSpeed);
}

// Get current speed
GET_DEFINE(MTR_GetSpeed, INT)
{
    Motor* pInstance = SM_GetInstance(Motor);
    return pInstance->currentSpeed;
}
