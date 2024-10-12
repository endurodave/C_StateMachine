#ifndef _FAULT_H
#define _FAULT_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning(disable : 4005)  // Disable warning C4005

// Used for compile-time checking for array sizes. On Windows VC++, you get 
// an "error C2118: negative subscript" error. On GCC: size of "unnamed array is negative"
#ifndef C_ASSERT
//#define C_ASSERT(expr)  {char uname[(expr)?1:-1];uname[0]=0;}		// Original macro
#define C_ASSERT(expr)  ((void)sizeof(char[(expr) ? 1 : -1]))		// New macro to fix GCC warning
#endif

#define ASSERT() \
	FaultHandler(__FILE__, (unsigned short) __LINE__)

#define ASSERT_TRUE(condition) \
	do {if (!(condition)) FaultHandler(__FILE__, (unsigned short) __LINE__);} while (0)

	/// Handles all software assertions in the system.
	/// @param[in] file - the file name that the software assertion occurred on
	/// @param[in] line - the line number that the software assertion occurred on
	void FaultHandler(const char* file, unsigned short line);

#ifdef __cplusplus
}
#endif

#endif 
