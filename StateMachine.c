#include "Fault.h"
#include "StateMachine.h"

// Generates an external event. Called once per external event 
// to start the state machine executing
void _SM_ExternalEvent(SM_StateMachine* self, BYTE newState, void* pEventData)
{
    ASSERT_TRUE(self);

    // If we are supposed to ignore this event
    if (newState == EVENT_IGNORED) 
    {
        // Just delete the event data, if any
        if (pEventData)
            SM_XFREE(pEventData);
    }
    else 
    {
        // TODO - capture software lock here for thread-safety if necessary

        // Generate the event and execute the state engine
        _SM_InternalEvent(self, newState, pEventData);
        _SM_StateEngine(self);

        // TODO - release software lock here 
    }
}

// Generates an internal event. Called from within a state 
// function to transition to a new state
void _SM_InternalEvent(SM_StateMachine* self, BYTE newState, void* pEventData)
{
    ASSERT_TRUE(self);

    self->pEventData = pEventData;
    self->eventGenerated = TRUE;
    self->currentState = newState;
}

// The state engine executes the state machine states
void _SM_StateEngine(SM_StateMachine* self)
{
    void* pDataTemp = NULL;

    ASSERT_TRUE(self);

    // While events are being generated keep executing states
    while (self->eventGenerated) 
    {
        pDataTemp = self->pEventData;  // copy of event data pointer
        self->pEventData = NULL;       // event data used up, reset ptr
        self->eventGenerated = FALSE;  // event used up, reset flag

        ASSERT_TRUE(self->currentState < self->maxStates);

        // Execute the state passing in event data, if any
        self->stateMap[self->currentState].pStateFunc(pDataTemp);

        // If event data was used, then delete it
        if (pDataTemp) 
        {
            SM_XFREE(pDataTemp);
            pDataTemp = NULL;
        }
    }
}
