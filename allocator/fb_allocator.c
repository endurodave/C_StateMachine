#include "fb_allocator.h"
#include "Fault.h"
#include <string.h>

// @see https://github.com/endurodave/C_Allocator
// David Lafreniere

// Lock options (std::mutex, FreeRTOS, ThreadX, Zephyr, CMSIS-RTOS2, none) are
// selected at compile time; see LockGuard.h. One lock guards every allocator's
// free-list and statistics.
#include "LockGuard.h"
static LOCK_HANDLE _hLock;

// Get a pointer to the client's area within a memory block
#define GET_CLIENT_PTR(_block_ptr_) \
    (_block_ptr_ ? ((void*)((char*)_block_ptr_)) : NULL)

// Get a pointer to the block using a client pointer
#define GET_BLOCK_PTR(_client_ptr_) \
    (_client_ptr_ ? ((void*)((char*)_client_ptr_)) : NULL)

static void* ALLOC_NewBlock(ALLOC_Allocator* alloc);
static void ALLOC_Push(ALLOC_Allocator* alloc, void* pBlock);
static void* ALLOC_Pop(ALLOC_Allocator* alloc);

//----------------------------------------------------------------------------
// ALLOC_NewBlock
// Get an unused block from the pool, or NULL. Caller holds the lock.
//----------------------------------------------------------------------------
static void* ALLOC_NewBlock(ALLOC_Allocator* self)
{
    ALLOC_Block* pBlock = NULL;

    // If we have not exceeded the pool maximum
    if (self->poolIndex < self->maxBlocks)
    {
        // Get pointer to a new fixed memory block within the pool
        pBlock = (ALLOC_Block*)(self->pPool + (self->poolIndex++ * self->blockSize));
    }

    return pBlock;
}

//----------------------------------------------------------------------------
// ALLOC_Push
// Push a block onto the free-list. Caller holds the lock.
//----------------------------------------------------------------------------
static void ALLOC_Push(ALLOC_Allocator* self, void* pBlock)
{
    // Get a pointer to the client's location within the block
    ALLOC_Block* pClient = (ALLOC_Block*)GET_CLIENT_PTR(pBlock);

    // Point client block's next pointer to head
    pClient->pNext = self->pHead;

    // The client block is now the new head
    self->pHead = pClient;
}

//----------------------------------------------------------------------------
// ALLOC_Pop
// Pop a block from the free-list, or NULL if empty. Caller holds the lock.
//----------------------------------------------------------------------------
static void* ALLOC_Pop(ALLOC_Allocator* self)
{
    ALLOC_Block* pBlock = NULL;

    // Is the free-list empty?
    if (self->pHead)
    {
        // Remove the head block
        pBlock = self->pHead;

        // Set the head to the next block
        self->pHead = (ALLOC_Block*)self->pHead->pNext;
    }

    return GET_BLOCK_PTR(pBlock);
}

//----------------------------------------------------------------------------
// ALLOC_Init
//----------------------------------------------------------------------------
void ALLOC_Init()
{
    _hLock = LK_CREATE();
}

//----------------------------------------------------------------------------
// ALLOC_Term
//----------------------------------------------------------------------------
void ALLOC_Term()
{
    LK_DESTROY(_hLock);
    _hLock = NULL;
}

//----------------------------------------------------------------------------
// ALLOC_Alloc
//----------------------------------------------------------------------------
void* ALLOC_Alloc(ALLOC_HANDLE hAlloc, size_t size)
{
    ALLOC_Allocator* self = NULL;
    void* pBlock = NULL;
    LOCK_STATE lock;

    ASSERT_TRUE(hAlloc);

    // Convert handle to an ALLOC_Allocator instance
    self = (ALLOC_Allocator*)hAlloc;

    // Ensure requested size fits within memory block
    ASSERT_TRUE(size <= self->blockSize);

    lock = LK_LOCK(_hLock);

    // Get a block from the free-list, or a new one from the pool
    pBlock = ALLOC_Pop(self);
    if (!pBlock)
        pBlock = ALLOC_NewBlock(self);

    if (pBlock)
    {
        // Keep track of usage statistics
        self->allocations++;
        self->blocksInUse++;
        if (self->blocksInUse > self->maxBlocksInUse)
        {
            self->maxBlocksInUse = self->blocksInUse;
        }
    }

    LK_UNLOCK(_hLock, lock);

    if (!pBlock)
    {
        // Out of fixed block memory
        ASSERT();
    }

    return GET_CLIENT_PTR(pBlock);
}

//----------------------------------------------------------------------------
// ALLOC_Calloc
//----------------------------------------------------------------------------
void* ALLOC_Calloc(ALLOC_HANDLE hAlloc, size_t num, size_t size)
{
    void* pMem = NULL;
    size_t n = 0;

    ASSERT_TRUE(hAlloc);

    // Compute the total size of the block
    n = num * size;

    // Allocate the memory
    pMem = ALLOC_Alloc(hAlloc, n);

    if (pMem != NULL)
    {
        // Initialize memory to 0 per calloc behavior
        memset(pMem, 0, n);
    }

    return pMem;
}

//----------------------------------------------------------------------------
// ALLOC_Free
//----------------------------------------------------------------------------
void ALLOC_Free(ALLOC_HANDLE hAlloc, void* pBlock)
{
    ALLOC_Allocator* self = NULL;
    LOCK_STATE lock;

    if (!pBlock)
        return;

    ASSERT_TRUE(hAlloc);

    // Cast handle to an allocator instance
    self = (ALLOC_Allocator*)hAlloc;

    // Get a pointer to the block
    pBlock = GET_BLOCK_PTR(pBlock);

    lock = LK_LOCK(_hLock);

    // Push the block onto a stack (i.e. the free-list)
    ALLOC_Push(self, pBlock);

    // Keep track of usage statistics
    self->deallocations++;
    self->blocksInUse--;

    LK_UNLOCK(_hLock, lock);
}
