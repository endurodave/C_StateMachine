// C_StateMachine: a compact finite state machine in C. Include this, or
// StateMachine.h directly.
//
// Event data comes from C_Allocator's fixed block pools: call ALLOC_Init()
// (fb_allocator.h, included here) once at startup, before the first event.
//
// @see https://github.com/endurodave/C_StateMachine

#ifndef C_STATE_MACHINE_H
#define C_STATE_MACHINE_H

#include "StateMachine.h"
#include "fb_allocator.h"

#endif // C_STATE_MACHINE_H
