#include "fb_allocator.h"
#include "DataTypes.h"
#include "Fault.h"
#include <string.h>

// Define USE_LOCK to use the default lock implementation
#define USE_LOCKS
#ifdef USE_LOCKS
    #include "LockGuard.h"
    static LOCK_HANDLE _hLock;
#else
    #pragma message("WARNING: Define software lock.")
    typedef int LOCK_HANDLE;
    static LOCK_HANDLE _hLock;

    #define LK_CREATE()     (1)
    #define LK_DESTROY(h)  
    #define LK_LOCK(h)    
    #define LK_UNLOCK(h)  
#endif

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
//----------------------------------------------------------------------------
static void* ALLOC_NewBlock(ALLOC_Allocator* self)
{
    ALLOC_Block* pBlock = NULL;

    LK_LOCK(_hLock);

    // If we have not exceeded the pool maximum
    if (self->poolIndex < self->maxBlocks)
    {
        // Get pointer to a new fixed memory block within the pool
        pBlock = (void*)(self->pPool + (self->poolIndex++ * self->blockSize));
    }

    LK_UNLOCK(_hLock);

    if (!pBlock)
    {
        // Out of fixed block memory
        ASSERT();
    }

    return pBlock;
} 

//----------------------------------------------------------------------------
// ALLOC_Push
//----------------------------------------------------------------------------
static void ALLOC_Push(ALLOC_Allocator* self, void* pBlock)
{
    if (!pBlock)
        return;

    // Get a pointer to the client's location within the block
    ALLOC_Block* pClient = (ALLOC_Block*)GET_CLIENT_PTR(pBlock);

    LK_LOCK(_hLock);

    // Point client block's next pointer to head
    pClient->pNext = self->pHead;

    // The client block is now the new head
    self->pHead = pClient;

    LK_UNLOCK(_hLock); 
}

//----------------------------------------------------------------------------
// ALLOC_Pop
//----------------------------------------------------------------------------
static void* ALLOC_Pop(ALLOC_Allocator* self)
{
    ALLOC_Block* pBlock = NULL;

    LK_LOCK(_hLock);

    // Is the free-list empty?
    if (self->pHead)
    {
        // Remove the head block
        pBlock = self->pHead;

        // Set the head to the next block
        self->pHead = self->pHead->pNext;
    }

    LK_UNLOCK(_hLock); 
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
}

//----------------------------------------------------------------------------
// ALLOC_Alloc
//----------------------------------------------------------------------------
void* ALLOC_Alloc(ALLOC_HANDLE hAlloc, size_t size)
{
    ALLOC_Allocator* self = NULL;
    void* pBlock = NULL;

    ASSERT_TRUE(hAlloc);

    // Convert handle to an ALLOC_Allocator instance
    self = (ALLOC_Allocator*)hAlloc;

    // Ensure requested size fits within memory block 
    ASSERT_TRUE(size <= self->blockSize);

    // Get a block from the free-list
    pBlock = ALLOC_Pop(self);

    // If the free-list empty?
    if (!pBlock)
    {
        // Get a new block from the pool
        pBlock = ALLOC_NewBlock(self);
    }

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

    if (!pBlock)
        return;

    ASSERT_TRUE(hAlloc);

    // Cast handle to an allocator instance
    self = (ALLOC_Allocator*)hAlloc;

    // Get a pointer to the block
    pBlock = GET_BLOCK_PTR(pBlock);

    // Push the block onto a stack (i.e. the free-list)
    ALLOC_Push(self, pBlock);

    // Keep track of usage statistics
    self->deallocations++;
    self->blocksInUse--;
} 



