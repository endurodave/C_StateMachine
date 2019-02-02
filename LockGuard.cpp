#include "LockGuard.h"
#include "Fault.h"
#include <mutex>

// A lock is a mutex
#define LOCK std::mutex

//------------------------------------------------------------------------------
// LK_Create
//------------------------------------------------------------------------------
LOCK_HANDLE LK_Create(void)
{
    LOCK* lock = new LOCK;
    return lock;
}

//------------------------------------------------------------------------------
// LK_Destroy
//------------------------------------------------------------------------------
void LK_Destroy(LOCK_HANDLE hLock)
{
    ASSERT_TRUE(hLock);
    LOCK* lock = (LOCK*)(hLock);
    delete lock;
}

//------------------------------------------------------------------------------
// LK_Lock
//------------------------------------------------------------------------------
void LK_Lock(LOCK_HANDLE hLock)
{
    ASSERT_TRUE(hLock);
    LOCK* lock = (LOCK*)(hLock);
	lock->lock();
}

//------------------------------------------------------------------------------
// LK_Unlock
//------------------------------------------------------------------------------
void LK_Unlock(LOCK_HANDLE hLock)
{
    ASSERT_TRUE(hLock);
    LOCK* lock = (LOCK*)(hLock);
    lock->unlock();
}

