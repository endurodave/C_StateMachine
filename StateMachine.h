#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H

#include <stdio.h>
#include "DataTypes.h"
#include "sm_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define USE_SM_ALLOCATOR to use the fixed block allocator instead of heap
#define USE_SM_ALLOCATOR
    #ifdef USE_SM_ALLOCATOR
    #define SM_XALLOC(size)    SMALLOC_Alloc(size)
    #define SM_XFREE(ptr)      SMALLOC_Free(ptr)
#else
    #include <stdlib.h>
    #define SM_XALLOC(size)    malloc(size)
    #define SM_XFREE(ptr)      free(ptr)
#endif

typedef void* SM_HANDLE;

#define EVENT_IGNORED  0xFE
#define CANNOT_HAPPEN  0xFF

// Generic state function signature
typedef void (*SM_StateFunc)(void*);

typedef struct
{
    SM_StateFunc pStateFunc;
} SM_StateStruct;

typedef struct 
{
    const char* name;
    const unsigned char maxStates;
    const SM_StateStruct* stateMap;
    unsigned char currentState;
    BOOL eventGenerated;
    void* pEventData;
} SM_StateMachine;

// Public functions
#define SM_ExternalEvent(_name_, _newState_, _data_) _SM_ExternalEvent(&_name_##Obj, _newState_, _data_)
#define SM_InternalEvent(_name_, _newState_, _data_) _SM_InternalEvent(&_name_##Obj, _newState_, _data_)

// Private functions
void _SM_ExternalEvent(SM_StateMachine* self, unsigned char newState, void* pData);
void _SM_InternalEvent(SM_StateMachine* self, unsigned char newState, void* pData);
void _SM_StateEngine(SM_StateMachine* self);

#define SM_DEFINE(_name_) \
    static SM_StateMachine _name_##Obj = { #_name_, \
        (sizeof(_name_##StateMap)/sizeof(_name_##StateMap[0])), \
        _name_##StateMap, 0, 0, 0 }; 

#define BEGIN_STATE_MAP(_name_) \
    static const SM_StateStruct _name_##StateMap[] = { 

#define STATE_MAP_ENTRY(stateFunc)\
    { stateFunc },

#define END_STATE_MAP \
    };

#define BEGIN_TRANSITION_MAP \
    static const unsigned char TRANSITIONS[] = {\

#define TRANSITION_MAP_ENTRY(entry)\
    entry,

#define END_TRANSITION_MAP(_name_, _data_) \
    0 };\
    _SM_ExternalEvent(&_name_##Obj, TRANSITIONS[_name_##Obj.currentState], _data_);

#ifdef __cplusplus
}
#endif
 
#endif // _STATE_MACHINE_H