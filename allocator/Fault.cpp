#include "Fault.h"
#include <assert.h>
#include <stdlib.h>
#if WIN32
	#include "windows.h"
#endif

// The default FaultHandler. On GCC and Clang it is a weak symbol: define your
// own FaultHandler() (e.g. to log and reset) and it replaces this one. That
// also lets libraries with their own FaultHandler (C_StateMachine,
// StateMachine) link together with this one.
#if defined(__GNUC__) || defined(__clang__)
	#define FAULT_WEAK __attribute__((weak))
#else
	#define FAULT_WEAK
#endif

//----------------------------------------------------------------------------
// FaultHandler
//----------------------------------------------------------------------------
FAULT_WEAK void FaultHandler(const char* file, unsigned short line)
{
	(void)file;
	(void)line;

#if WIN32
	// If you hit this line, it means one of the ASSERT macros failed.
    DebugBreak();
#endif

	assert(0);

	// assert() does nothing with NDEBUG; a fault must not return
	abort();
}
