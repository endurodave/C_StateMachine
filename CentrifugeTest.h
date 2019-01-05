#ifndef _CENTRIFUGE_TEST_H
#define _CENTRIFUGE_TEST_H

#include "DataTypes.h"
#include "StateMachine.h"

// Declare the private instance of CentrifugeTest state machine
SM_DECLARE(CentrifugeTestSM)

// State machine event functions
EVENT_DECLARE(CFG_Start, NoEventData)
EVENT_DECLARE(CFG_Cancel, NoEventData)
EVENT_DECLARE(CFG_Poll, NoEventData)

BOOL CFG_IsPollActive();

#endif // _CENTRIFUGE_TEST_H
