#include "fb_allocator.h"
#include "StateMachine.h"
#include "Motor.h"
#include "CentrifugeTest.h"

// @see https://github.com/endurodave/C_StateMachine
// 
// Other related repos:
// @see https://github.com/endurodave/C_StateMachineWithThreads
// @see https://github.com/endurodave/C_Allocator

// Define motor objects
static Motor motorObj1;
static Motor motorObj2;

// Define two public Motor state machine instances
SM_DEFINE(Motor1SM, &motorObj1)
SM_DEFINE(Motor2SM, &motorObj2)

int main(void)
{
    ALLOC_Init();

    MotorData* data;

    // Create event data
    data = SM_XAlloc(sizeof(MotorData));
    data->speed = 100;

    // Call MTR_SetSpeed event function to start motor
    SM_Event(Motor1SM, MTR_SetSpeed, data);

    // Call MTR_SetSpeed event function to change motor speed
    data = SM_XAlloc(sizeof(MotorData));
    data->speed = 200;
    SM_Event(Motor1SM, MTR_SetSpeed, data);

    // Get current speed from Motor1SM
    INT currentSpeed = SM_Get(Motor1SM, MTR_GetSpeed);

    // Stop motor again will be ignored
    SM_Event(Motor1SM, MTR_Halt, NULL);

    // Motor2SM example
    data = SM_XAlloc(sizeof(MotorData));
    data->speed = 300;
    SM_Event(Motor2SM, MTR_SetSpeed, data);
    SM_Event(Motor2SM, MTR_Halt, NULL);

    // CentrifugeTestSM example
    SM_Event(CentrifugeTestSM, CFG_Cancel, NULL);
    SM_Event(CentrifugeTestSM, CFG_Start, NULL);
    while (CFG_IsPollActive())
        SM_Event(CentrifugeTestSM, CFG_Poll, NULL);

    ALLOC_Term();

    return 0;
}

