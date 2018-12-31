#include "x_allocator.h"
#include "fb_allocator.h"
#include "DataTypes.h"
#include "Fault.h"
#include <string.h>

static void* XALLOC_PutAllocatorPtrInBlock(void* block, ALLOC_Allocator* allocator);
static ALLOC_Allocator* XALLOC_GetAllocatorPtrFromBlock(void* block);
static ALLOC_Allocator* XALLOC_GetAllocator(XAllocData* self, size_t size);

//----------------------------------------------------------------------------
// XALLOC_PutAllocatorPtrInBlock
//----------------------------------------------------------------------------
static void* XALLOC_PutAllocatorPtrInBlock(void* block, ALLOC_Allocator* allocator)
{
    ALLOC_Allocator** pAllocatorInBlock;

    ASSERT_TRUE(block);
    ASSERT_TRUE(allocator);

    // Cast raw block memory to ALLOC_Allocator**
    pAllocatorInBlock = (ALLOC_Allocator**)(block);

    // Store the allocator pointer in the memory block 
    *pAllocatorInBlock = allocator;

    // Advance the pointer past the ALLOC_Allocator* and return a
    // pointer to the client's memory region
    return ++pAllocatorInBlock;
}

//----------------------------------------------------------------------------
// XALLOC_GetAllocatorPtrFromBlock
//----------------------------------------------------------------------------
static ALLOC_Allocator* XALLOC_GetAllocatorPtrFromBlock(void* block)
{
    ALLOC_Allocator** pAllocatorInBlock;

    ASSERT_TRUE(block);

    // Cast raw memory block to ALLOC_Allocator**
    pAllocatorInBlock = (ALLOC_Allocator**)(block);

    // Backup one ALLOC_Allocator* position to get the stored allocator instance
    pAllocatorInBlock--;

    // Return the allocator instance stored within the memory block
    return *pAllocatorInBlock;
}

//----------------------------------------------------------------------------
// XALLOC_GetBlockPtr
//----------------------------------------------------------------------------
static void* XALLOC_GetBlockPtr(void* block)
{
    ALLOC_Allocator** pAllocatorInBlock;

    ASSERT_TRUE(block);

    // Cast the client memory to ALLOC_Allocator* 
    pAllocatorInBlock = (ALLOC_Allocator**)(block);

    // Back up one ALLOC_Allocator* position and return raw memory block pointer
    return --pAllocatorInBlock;
}

//----------------------------------------------------------------------------
// XALLOC_GetAllocator
//----------------------------------------------------------------------------
static ALLOC_Allocator* XALLOC_GetAllocator(XAllocData* self, size_t size)
{
    UINT16 i = 0;
    ALLOC_Allocator* pAllocator = NULL;

    ASSERT_TRUE(self);

    // Each block stores additional meta data (i.e. an ALLOC_Allocator pointer). 
    // Add overhead for the additional memory required.
    size += XALLOC_BLOCK_META_DATA_SIZE;

    // Iterate over all allocators 
    for (i=0; i<self->maxAllocators; i++)
    {
        // Can the allocator instance handle the requested size?
        if (self->allocators[i] && self->allocators[i]->blockSize >= size)
        {
            // Return allocator instance to handle memory request
            pAllocator = self->allocators[i];
            break;
        }
    }

    return pAllocator;
} 

//----------------------------------------------------------------------------
// XALLOC_Alloc
//----------------------------------------------------------------------------
void* XALLOC_Alloc(XAllocData* self, size_t size)
{
    ALLOC_Allocator* pAllocator;
    void* pBlockMemory = NULL;
    void* pClientMemory = NULL;

    ASSERT_TRUE(self);

    // Get an allocator instance to handle the memory request
    pAllocator = XALLOC_GetAllocator(self, size);

    // An allocator found to handle memory request?
    if (pAllocator)
    {
        // Get a fixed memory block from the allocator instance
        pBlockMemory = ALLOC_Alloc(pAllocator, size + XALLOC_BLOCK_META_DATA_SIZE);
        if (pBlockMemory)
        {
            // Set the block ALLOC_Allocator* ptr within the raw memory block region
            pClientMemory = XALLOC_PutAllocatorPtrInBlock(pBlockMemory, pAllocator);
        }
    }
    else
    {
        // Too large a memory block requested
        ASSERT();
    }

    return pClientMemory;
} 

//----------------------------------------------------------------------------
// XALLOC_Free
//----------------------------------------------------------------------------
void XALLOC_Free(void* ptr)
{
    ALLOC_Allocator* pAllocator = NULL;
    void* pBlock = NULL;

    if (!ptr)
        return;

    // Extract the original allocator instance from the caller's block pointer
    pAllocator = XALLOC_GetAllocatorPtrFromBlock(ptr);
    if (pAllocator)
    {
        // Convert the client pointer into the original raw block pointer
        pBlock = XALLOC_GetBlockPtr(ptr);

        // Deallocate the fixed memory block
        ALLOC_Free(pAllocator, pBlock);
    }
} 

//----------------------------------------------------------------------------
// XALLOC_Realloc
//----------------------------------------------------------------------------
void* XALLOC_Realloc(XAllocData* self, void *ptr, size_t new_size)
{
    void* pNewMem = NULL;
    ALLOC_Allocator* pOldAllocator = NULL;
    size_t oldSize = 0;

    ASSERT_TRUE(self);

    if (!ptr)
        pNewMem = XALLOC_Alloc(self, new_size);
    else if (0 == new_size)
        XALLOC_Free(ptr);
    else
    {
        // Create a new memory block
        pNewMem = XALLOC_Alloc(self, new_size);
        if (pNewMem != 0)
        {
            // Get the original allocator instance from the old memory block
            pOldAllocator = XALLOC_GetAllocatorPtrFromBlock(ptr);
            oldSize = pOldAllocator->blockSize - XALLOC_BLOCK_META_DATA_SIZE;

            // Copy the bytes from the old memory block into the new (as much as will fit)
            memcpy(pNewMem, ptr, (oldSize < new_size) ? oldSize : new_size);

            // Free the old memory block
            XALLOC_Free(ptr);
        }
    }

    // Return the client pointer to the new memory block
    return pNewMem;
} 

//----------------------------------------------------------------------------
// XALLOC_Calloc
//----------------------------------------------------------------------------
void* XALLOC_Calloc(XAllocData* self, size_t num, size_t size)
{
    void* pMem = NULL;
    size_t n;

    ASSERT_TRUE(self);

    // Compute the total block size
    n = num * size;

    // Allocate the memory
    pMem = XALLOC_Alloc(self, n);

    if (pMem)
    {
        // Initialize memory to 0
        memset(pMem, 0, n);
    }

    return pMem;
} 



