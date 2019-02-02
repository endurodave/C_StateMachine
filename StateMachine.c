#include "Fault.h"
#include "StateMachine.h"

// Generates an external event. Called once per external event 
// to start the state machine executing
void _SM_ExternalEvent(SM_StateMachine* self, const SM_StateMachineConst* selfConst, BYTE newState, void* pEventData)
{
    // If we are supposed to ignore this event
    if (newState == EVENT_IGNORED) 
    {
        // Just delete the event data, if any
        if (pEventData)
            SM_XFree(pEventData);
    }
    else 
    {
        // TODO - capture software lock here for thread-safety if necessary

        // Generate the event 
        _SM_InternalEvent(self, newState, pEventData);

        // Execute state machine based on type of state map defined
        if (selfConst->stateMap)
            _SM_StateEngine(self, selfConst);
        else
            _SM_StateEngineEx(self, selfConst);

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
    self->newState = newState;
}

// The state engine executes the state machine states
void _SM_StateEngine(SM_StateMachine* self, const SM_StateMachineConst* selfConst)
{
    void* pDataTemp = NULL;

    ASSERT_TRUE(self);
    ASSERT_TRUE(selfConst);

    // While events are being generated keep executing states
    while (self->eventGenerated)
    {
        // Error check that the new state is valid before proceeding
        ASSERT_TRUE(self->newState < selfConst->maxStates);

        // Get the pointers from the state map
        SM_StateFunc state = selfConst->stateMap[self->newState].pStateFunc;

        // Copy of event data pointer
        pDataTemp = self->pEventData;

        // Event data used up, reset the pointer
        self->pEventData = NULL;

        // Event used up, reset the flag
        self->eventGenerated = FALSE;

        // Switch to the new current state
        self->currentState = self->newState;

        // Execute the state action passing in event data
        ASSERT_TRUE(state != NULL);
        state(self, pDataTemp);

        // If event data was used, then delete it
        if (pDataTemp)
        {
            SM_XFree(pDataTemp);
            pDataTemp = NULL;
        }
    }
}

// The state engine executes the extended state machine states
void _SM_StateEngineEx(SM_StateMachine* self, const SM_StateMachineConst* selfConst)
{
    BOOL guardResult = TRUE;
    void* pDataTemp = NULL;

    ASSERT_TRUE(self);
    ASSERT_TRUE(selfConst);

    // While events are being generated keep executing states
    while (self->eventGenerated)
    {
        // Error check that the new state is valid before proceeding
        ASSERT_TRUE(self->newState < selfConst->maxStates);

        // Get the pointers from the extended state map
        SM_StateFunc state = selfConst->stateMapEx[self->newState].pStateFunc;
        SM_GuardFunc guard = selfConst->stateMapEx[self->newState].pGuardFunc;
        SM_EntryFunc entry = selfConst->stateMapEx[self->newState].pEntryFunc;
        SM_ExitFunc exit = selfConst->stateMapEx[self->currentState].pExitFunc;

        // Copy of event data pointer
        pDataTemp = self->pEventData;

        // Event data used up, reset the pointer
        self->pEventData = NULL;

        // Event used up, reset the flag
        self->eventGenerated = FALSE;

        // Execute the guard condition
        if (guard != NULL)
            guardResult = guard(self, pDataTemp);

        // If the guard condition succeeds
        if (guardResult == TRUE)
        {
            // Transitioning to a new state?
            if (self->newState != self->currentState)
            {
                // Execute the state exit action on current state before switching to new state
                if (exit != NULL)
                    exit(self);

                // Execute the state entry action on the new state
                if (entry != NULL)
                    entry(self, pDataTemp);

                // Ensure exit/entry actions didn't call SM_InternalEvent by accident 
                ASSERT_TRUE(self->eventGenerated == FALSE);
            }

            // Switch to the new current state
            self->currentState = self->newState;

            // Execute the state action passing in event data
            ASSERT_TRUE(state != NULL);
            state(self, pDataTemp);
        }

        // If event data was used, then delete it
        if (pDataTemp)
        {
            SM_XFree(pDataTemp);
            pDataTemp = NULL;
        }
    }
}
