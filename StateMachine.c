#include "Fault.h"
#include "StateMachine.h"

void _SM_ExternalEvent(SM_StateMachine* self, unsigned char newState, void* pData)
{
    // If we are supposed to ignore this event
    if (newState == EVENT_IGNORED) 
    {
        // Just delete the event data, if any
        if (pData)
            SM_XFREE(pData);
    }
    else 
    {
        // TODO - capture software lock here for thread-safety if necessary

        // Generate the event and execute the state engine
        _SM_InternalEvent(self, newState, pData);
        _SM_StateEngine(self);

        // TODO - release software lock here 
    }
}

void _SM_InternalEvent(SM_StateMachine* self, unsigned char newState, void* pData)
{
    self->pEventData = pData;
    self->eventGenerated = TRUE;
    self->currentState = newState;
}

void _SM_StateEngine(SM_StateMachine* self)
{
    void* pDataTemp = NULL;

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
