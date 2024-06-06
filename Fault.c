#include "Fault.h"
#include <assert.h>
#if defined(_WIN32) || defined(_WIN64)
	#include "windows.h"
#endif

//----------------------------------------------------------------------------
// FaultHandler
//----------------------------------------------------------------------------
void FaultHandler(const char* file, unsigned short line)
{
#if defined(_WIN32) || defined(_WIN64)
	// If you hit this line, it means one of the ASSERT macros failed.
    DebugBreak();
#endif
	assert(0);
}
