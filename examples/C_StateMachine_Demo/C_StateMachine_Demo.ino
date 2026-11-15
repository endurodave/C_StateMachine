// C_StateMachine demo: a motor state machine driven by external events, with
// event data from C_Allocator's fixed block pools (SM_XAlloc), no heap.
// Results are printed on Serial at 115200.
//
// Motor.c / Motor.h in this folder define the state machine:
//   Idle -> Start -> ChangeSpeed -> Stop -> Idle
//
// @see https://github.com/endurodave/C_StateMachine

#include <C_StateMachine.h>
#include "Motor.h"

// Motor.c logs each state change through this
extern "C" void DemoLog(const char* msg)
{
    Serial.println(msg);
}

// Two motors, each with its own state machine instance
static Motor motorObj1;
static Motor motorObj2;
SM_DEFINE(Motor1SM, &motorObj1)
SM_DEFINE(Motor2SM, &motorObj2)

static void SetSpeed(SM_StateMachine* sm, INT speed)
{
    // Event data is allocated with SM_XAlloc; the state machine frees it
    MotorData* data = (MotorData*)SM_XAlloc(sizeof(MotorData));
    data->speed = speed;
    MTR_SetSpeed(sm, data);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("C_StateMachine demo");
    ALLOC_Init();

    bool ok = true;
    SetSpeed(&Motor1SMObj, 100);                   // Idle -> Start
    SetSpeed(&Motor1SMObj, 200);                   // Start -> ChangeSpeed
    ok = ok && SM_Get(Motor1SM, MTR_GetSpeed) == 200;
    SM_Event(Motor1SM, MTR_Halt, NULL);            // ChangeSpeed -> Stop -> Idle
    SM_Event(Motor1SM, MTR_Halt, NULL);            // ignored in Idle
    ok = ok && SM_Get(Motor1SM, MTR_GetSpeed) == 0;

    SetSpeed(&Motor2SMObj, 300);
    ok = ok && SM_Get(Motor2SM, MTR_GetSpeed) == 300;
    SM_Event(Motor2SM, MTR_Halt, NULL);

    Serial.println(ok ? "C_StateMachine demo PASSED" : "C_StateMachine demo FAILED");
}

void loop()
{
}
