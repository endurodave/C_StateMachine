// SMALLOC allocates either a 32 or 128 byte block depending 
// on the requested size. 

#include "sm_allocator.h"
#include "x_allocator.h"

// Maximum number of blocks for each size
#define MAX_32_BLOCKS   10
#define MAX_128_BLOCKS	5

// Define size of each block including meta data overhead
#define BLOCK_32_SIZE     32 + XALLOC_BLOCK_META_DATA_SIZE
#define BLOCK_128_SIZE    128 + XALLOC_BLOCK_META_DATA_SIZE

// Define individual fb_allocators
ALLOC_DEFINE(smDataAllocator32, BLOCK_32_SIZE, MAX_32_BLOCKS)
ALLOC_DEFINE(smDataAllocator128, BLOCK_128_SIZE, MAX_128_BLOCKS)

// An array of allocators sorted by smallest block first
static ALLOC_Allocator* allocators[] = {
    &smDataAllocator32Obj,
    &smDataAllocator128Obj
};

#define MAX_ALLOCATORS   (sizeof(allocators) / sizeof(allocators[0]))

static XAllocData self = { allocators, MAX_ALLOCATORS };

//----------------------------------------------------------------------------
// SMALLOC_Alloc
//----------------------------------------------------------------------------
void* SMALLOC_Alloc(size_t size)
{
    return XALLOC_Alloc(&self, size);
}

//----------------------------------------------------------------------------
// SMALLOC_Free
//----------------------------------------------------------------------------
void SMALLOC_Free(void* ptr)
{
    XALLOC_Free(ptr);
}

//----------------------------------------------------------------------------
// SMALLOC_Realloc
//----------------------------------------------------------------------------
void* SMALLOC_Realloc(void *ptr, size_t new_size)
{
    return XALLOC_Realloc(&self, ptr, new_size);
}

//----------------------------------------------------------------------------
// SMALLOC_Calloc
//----------------------------------------------------------------------------
void* SMALLOC_Calloc(size_t num, size_t size)
{
    return XALLOC_Calloc(&self, num, size);
}

