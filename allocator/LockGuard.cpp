#include "LockGuard.h"
#include "Fault.h"
// A C++ header first, so __GLIBCXX__ is defined below (AVR has only C headers)
#if defined(__AVR__)
	#include <stddef.h>
#else
	#include <cstddef>
#endif

// @see https://github.com/endurodave/C_Allocator
// David Lafreniere
//
// Lock options are described in LockGuard.h.

#if defined(SM_NO_LOCKS) && !defined(ALLOC_LOCK_NONE)
	#define ALLOC_LOCK_NONE
#endif

#if !defined(ALLOC_LOCK_FREERTOS) && !defined(ALLOC_LOCK_THREADX) && \
	!defined(ALLOC_LOCK_ZEPHYR) && !defined(ALLOC_LOCK_CMSIS_RTOS2) && \
	!defined(ALLOC_LOCK_USER) && !defined(ALLOC_LOCK_NONE)
	// No std::mutex: AVR has no C++ standard library, and libstdc++ built
	// without threads support (bare-metal arm-none-eabi) has no <mutex>
	#if defined(__AVR__) || (defined(__GLIBCXX__) && !defined(_GLIBCXX_HAS_GTHREADS))
		#define ALLOC_LOCK_NONE
	#endif
#endif

#if (defined(ALLOC_LOCK_FREERTOS) + defined(ALLOC_LOCK_THREADX) + \
	defined(ALLOC_LOCK_ZEPHYR) + defined(ALLOC_LOCK_CMSIS_RTOS2) + \
	defined(ALLOC_LOCK_USER) + defined(ALLOC_LOCK_NONE)) > 1
	#error "Define only one ALLOC_LOCK_ option"
#endif

#if defined(ALLOC_LOCK_FREERTOS) || defined(ALLOC_LOCK_THREADX) || \
	defined(ALLOC_LOCK_ZEPHYR) || defined(ALLOC_LOCK_CMSIS_RTOS2) || \
	defined(ALLOC_LOCK_USER) || defined(ALLOC_LOCK_NONE)
	// One lock for all allocators, with nothing to create: the handle only
	// records that ALLOC_Init() was called
	static char _lockToken;
	LOCK_HANDLE LK_Create(void) { return &_lockToken; }
	void LK_Destroy(LOCK_HANDLE hLock) { ASSERT_TRUE(hLock); }
#endif

#if defined(ALLOC_LOCK_FREERTOS)
	#if defined(ESP_PLATFORM)
		#include "freertos/FreeRTOS.h"
		#include "freertos/task.h"
		// ESP32 cores share one spinlock; the _SAFE forms work in tasks and ISRs
		static portMUX_TYPE _allocMux = portMUX_INITIALIZER_UNLOCKED;
		LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
		{
			ASSERT_TRUE(hLock);
			portENTER_CRITICAL_SAFE(&_allocMux);
			return 0;
		}
		void LK_Unlock(LOCK_HANDLE, LOCK_STATE) { portEXIT_CRITICAL_SAFE(&_allocMux); }
	#else
		#include "FreeRTOS.h"
		#include "task.h"
		// LOCK_STATE low 2 bits: how the lock was taken; the rest: saved mask
		enum { LK_NONE = 0, LK_TASK = 1, LK_ISR = 2 };
		LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
		{
			ASSERT_TRUE(hLock);
		#if (INCLUDE_xTaskGetSchedulerState == 1) || (configUSE_TIMERS == 1)
			// Before the scheduler starts there is one thread of execution.
			// A FreeRTOS critical section here would leave interrupts disabled
			// until the scheduler starts, stalling e.g. Arduino delay() and Serial.
			if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
				return LK_NONE;
		#endif
		#if defined(__arm__) || defined(__ARM_ARCH)
			if (xPortIsInsideInterrupt() != pdFALSE)
				return ((LOCK_STATE)taskENTER_CRITICAL_FROM_ISR() << 2) | LK_ISR;
		#endif
			taskENTER_CRITICAL();
			return LK_TASK;
		}
		void LK_Unlock(LOCK_HANDLE, LOCK_STATE state)
		{
			if ((state & 3) == LK_ISR)
				taskEXIT_CRITICAL_FROM_ISR((UBaseType_t)(state >> 2));
			else if ((state & 3) == LK_TASK)
				taskEXIT_CRITICAL();
		}
	#endif
#elif defined(ALLOC_LOCK_THREADX)
	#include "tx_api.h"
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		return (LOCK_STATE)tx_interrupt_control(TX_INT_DISABLE);
	}
	void LK_Unlock(LOCK_HANDLE, LOCK_STATE state) { tx_interrupt_control((UINT)state); }
#elif defined(ALLOC_LOCK_ZEPHYR)
	#include <zephyr/kernel.h>
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		return (LOCK_STATE)irq_lock();
	}
	void LK_Unlock(LOCK_HANDLE, LOCK_STATE state) { irq_unlock((unsigned int)state); }
#elif defined(ALLOC_LOCK_CMSIS_RTOS2)
	#include "cmsis_compiler.h"
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		LOCK_STATE state = (LOCK_STATE)__get_PRIMASK();
		__disable_irq();
		return state;
	}
	void LK_Unlock(LOCK_HANDLE, LOCK_STATE state) { __set_PRIMASK((uint32_t)state); }
#elif defined(ALLOC_LOCK_USER)
	extern "C" void ALLOC_UserLock(void);
	extern "C" void ALLOC_UserUnlock(void);
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		ALLOC_UserLock();
		return 0;
	}
	void LK_Unlock(LOCK_HANDLE, LOCK_STATE) { ALLOC_UserUnlock(); }
#elif defined(ALLOC_LOCK_NONE)
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		return 0;
	}
	void LK_Unlock(LOCK_HANDLE, LOCK_STATE) {}
#else
	#include <mutex>

	//------------------------------------------------------------------------------
	// LK_Create
	//------------------------------------------------------------------------------
	LOCK_HANDLE LK_Create(void)
	{
		return new std::mutex;
	}

	//------------------------------------------------------------------------------
	// LK_Destroy
	//------------------------------------------------------------------------------
	void LK_Destroy(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		delete static_cast<std::mutex*>(hLock);
	}

	//------------------------------------------------------------------------------
	// LK_Lock
	//------------------------------------------------------------------------------
	LOCK_STATE LK_Lock(LOCK_HANDLE hLock)
	{
		ASSERT_TRUE(hLock);
		static_cast<std::mutex*>(hLock)->lock();
		return 0;
	}

	//------------------------------------------------------------------------------
	// LK_Unlock
	//------------------------------------------------------------------------------
	void LK_Unlock(LOCK_HANDLE hLock, LOCK_STATE)
	{
		ASSERT_TRUE(hLock);
		static_cast<std::mutex*>(hLock)->unlock();
	}
#endif
