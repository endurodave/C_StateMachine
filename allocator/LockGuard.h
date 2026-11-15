#ifndef _LOCK_GUARD_H
#define _LOCK_GUARD_H

// @see https://github.com/endurodave/C_Allocator
// David Lafreniere
//
// Software lock for the fb_allocator free-lists. It is held only for a
// free-list push or pop, so on an RTOS a short critical section fits better
// than a mutex: no kernel object, no priority inversion, and it works before
// the scheduler starts. Selected at compile time in LockGuard.cpp; define at
// most one option for the whole build:
//
//   (default)               std::mutex, when the C++ library provides it
//                           (Windows, Linux, macOS, ESP32); otherwise none.
//   ALLOC_LOCK_FREERTOS     FreeRTOS critical section (taskENTER_CRITICAL, or
//                           the _FROM_ISR form inside an interrupt on Cortex-M;
//                           portENTER_CRITICAL_SAFE spinlock on ESP32).
//   ALLOC_LOCK_THREADX      ThreadX tx_interrupt_control() interrupt masking.
//   ALLOC_LOCK_ZEPHYR       Zephyr irq_lock()/irq_unlock().
//   ALLOC_LOCK_CMSIS_RTOS2  Cortex-M PRIMASK interrupt masking (CMSIS-Core),
//                           for any CMSIS-RTOS2 kernel (RTX5, FreeRTOS, ...).
//   ALLOC_LOCK_USER         you define void ALLOC_UserLock(void) and
//                           void ALLOC_UserUnlock(void).
//   ALLOC_LOCK_NONE         no lock: single-threaded use only. SM_NO_LOCKS
//                           (C_StateMachine) selects it too.
//
// Bare-metal toolchains without threads support (e.g. arm-none-eabi as used by
// STM32duino and Arduino-Pico, or AVR) have no std::mutex, so there the default
// is ALLOC_LOCK_NONE: pick an RTOS option if the allocators are used from more
// than one task. With an interrupt-masking option (FreeRTOS on Cortex-M or
// ESP32, ThreadX, Zephyr, CMSIS-RTOS2), ALLOC_Alloc()/ALLOC_Free() may also be
// called from an interrupt.

#ifdef __cplusplus
extern "C" {
#endif

typedef void* LOCK_HANDLE;

/// State saved by LK_LOCK() and restored by LK_UNLOCK(), e.g. the interrupt
/// mask. Kept by the caller, so nested and interrupt-context locks each
/// restore their own state.
typedef unsigned long LOCK_STATE;

#define LK_CREATE()         LK_Create()
#define LK_DESTROY(h)       LK_Destroy(h)
#define LK_LOCK(h)          LK_Lock(h)
#define LK_UNLOCK(h, s)     LK_Unlock(h, s)

LOCK_HANDLE LK_Create(void);
void LK_Destroy(LOCK_HANDLE hLock);
LOCK_STATE LK_Lock(LOCK_HANDLE hLock);
void LK_Unlock(LOCK_HANDLE hLock, LOCK_STATE state);

#ifdef __cplusplus
}
#endif

#endif
