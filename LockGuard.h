#ifndef _LOCK_GUARD_H
#define _LOCK_GUARD_H

#include "DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* LOCK_HANDLE;

#define LK_CREATE()     LK_Create()
#define LK_DESTROY(h)   LK_Destroy(h)
#define LK_LOCK(h)      LK_Lock(h)
#define LK_UNLOCK(h)    LK_Unlock(h)

LOCK_HANDLE LK_Create(void);
void LK_Destroy(LOCK_HANDLE hLock);
void LK_Lock(LOCK_HANDLE hLock);
void LK_Unlock(LOCK_HANDLE hLock);

#ifdef __cplusplus
}
#endif

#endif 
