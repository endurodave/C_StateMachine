#include "Motor.h"

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
static void ST_Idle(void* data);
static void ST_Stop(void* data);
static void ST_Start(MotorData* data);
static void ST_ChangeSpeed(MotorData* data);

// State map to define state function order
BEGIN_STATE_MAP(Motor)
    STATE_MAP_ENTRY(ST_Idle)
    STATE_MAP_ENTRY(ST_Stop)
    STATE_MAP_ENTRY(ST_Start)
    STATE_MAP_ENTRY(ST_ChangeSpeed)
END_STATE_MAP

// Define the Motor state machine
SM_DEFINE(Motor)

// Set motor speed external event
void MTR_SetSpeed(MotorData* data)
{
    // Given the SetSpeed event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                     // - Current State -
        TRANSITION_MAP_ENTRY(ST_START)       // ST_Idle       
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)  // ST_Stop       
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)// ST_Start      
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED)// ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, data)
}

// Halt motor external event
void MTR_Halt(void)
{
    // Given the Halt event, transition to a new state based upon 
    // the current state of the state machine
    BEGIN_TRANSITION_MAP                     // - Current State -
        TRANSITION_MAP_ENTRY(EVENT_IGNORED)  // ST_Idle
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN)  // ST_Stop
        TRANSITION_MAP_ENTRY(ST_STOP)        // ST_Start
        TRANSITION_MAP_ENTRY(ST_STOP)        // ST_ChangeSpeed
    END_TRANSITION_MAP(Motor, NULL)
}

// State machine sits here when motor is not running
static void ST_Idle(void* data)
{
    printf("ST_Idle\n");
}

// Stop the motor 
static void ST_Stop(void* data)
{
    printf("ST_Stop\n");

    // Perform the stop motor processing here
    // Transition to ST_Idle via an internal event
    SM_InternalEvent(Motor, ST_IDLE, NULL);
}

// Start the motor going
static void ST_Start(MotorData* data)
{
    printf("ST_Start %d\n", data->speed);
    // Set initial motor speed processing here
}

// Changes the motor speed once the motor is moving
static void ST_ChangeSpeed(MotorData* data)
{
    printf("ST_ChangeSpeed %d\n", data->speed);
    // Perform the change motor speed to pData->speed here
}

