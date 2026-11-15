// C_StateMachine PlatformIO example: a motor state machine whose event data
// comes from C_Allocator's fixed block pools (SM_XAlloc), with no heap.
// Runs on Arduino boards and the desktop (native).
//
// @see https://github.com/endurodave/C_StateMachine
// @see https://github.com/endurodave/C_Allocator

#include "fb_allocator.h"
#include "StateMachine.h"
#include "Motor.h"
#include <stdio.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Set while Stress() runs, so its 1000 state changes are not logged
static bool quiet = false;

extern "C" void DemoLog(const char* msg)
{
    if (quiet)
        return;
#ifdef ARDUINO
    Serial.println(msg);
#else
    printf("%s\n", msg);
#endif
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

static bool Demo()
{
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

    return ok;
}

// Many events through the fixed block pools: each event's data must be
// freed again, or the small pools run out and the allocator faults
static bool Stress()
{
    quiet = true;
    for (INT i = 1; i <= 1000; i++)
    {
        MotorData* data = (MotorData*)SM_XAlloc(sizeof(MotorData));
        data->speed = i;
        MTR_SetSpeed(&Motor2SMObj, data);
        if (SM_Get(Motor2SM, MTR_GetSpeed) != i)
        {
            quiet = false;
            return false;
        }
    }
    quiet = false;
    char buf[48];
    snprintf(buf, sizeof(buf), "1000 events, last speed %d", (int)SM_Get(Motor2SM, MTR_GetSpeed));
    DemoLog(buf);
    return true;
}

static bool Run()
{
    ALLOC_Init();
    bool ok = Demo();
    ok = ok && Stress();
    DemoLog(ok ? "C_StateMachine demo PASSED" : "C_StateMachine demo FAILED");
    return ok;
}

#ifdef ARDUINO
void setup()
{
    Serial.begin(115200);
    delay(1000);
    DemoLog("C_StateMachine demo");
    Run();
}

void loop()
{
}
#else
int main()
{
    DemoLog("C_StateMachine demo");
    bool ok = Run();
    ALLOC_Term();
    return ok ? 0 : 1;
}
#endif
