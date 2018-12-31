// https://www.codeproject.com/Articles/1272619/A-Fixed-Block-Memory-Allocator-in-C
//
// The fb_allocator is a fixed block memory allocator that handles a 
// single block size. 
//
// Create an allocator instance using the ALLOC_DEFINE macro. Call 
// ALLOC_Init() one time at startup. ALLOC_Alloc() allocates a fixed 
// memory block. ALLOC_Free() frees the block. 
//
// #include "fb_allocator.h"
// ALLOC_DEFINE(myAllocator, 32, 5)
//
// void main() 
// {
//      void* block;
//      ALLOC_Init();
//      block = ALLOC_Alloc(myAllocator, 32);
//      ALLOC_Free(myAllocator, block);
// }

#ifndef _FB_ALLOCATOR_H
#define _FB_ALLOCATOR_H

#include <stdlib.h>
#include "DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ALLOC_HANDLE;

typedef struct 
{
    void* pNext;
} ALLOC_Block;

// Use ALLOC_DEFINE to declare an ALLOC_Allocator object
typedef struct
{
    const char* name;
    const char* pPool;
    const size_t objectSize;
    const size_t blockSize;
    const UINT32 maxBlocks;
    ALLOC_Block* pHead;
    UINT16 poolIndex;
    UINT16 blocksInUse;
    UINT16 maxBlocksInUse;
    UINT16 allocations;
    UINT16 deallocations;
} ALLOC_Allocator;

// Align fixed blocks on X-byte boundary based on CPU architecture.
// Set value to 1, 2, 4 or 8.
#define ALLOC_MEM_ALIGN   (1)

// Get the maximum between a or b
#define ALLOC_MAX(a,b) (((a)>(b))?(a):(b))

// Round _numToRound_ to the next higher _multiple_
#define ALLOC_ROUND_UP(_numToRound_, _multiple_) \
    (((_numToRound_ + _multiple_ - 1) / _multiple_) * _multiple_)

// Ensure the memory block size is: (a) is aligned on desired boundary and (b) at
// least the size of a ALLOC_Allocator*. 
#define ALLOC_BLOCK_SIZE(_size_) \
    (ALLOC_MAX((ALLOC_ROUND_UP(_size_, ALLOC_MEM_ALIGN)), sizeof(ALLOC_Allocator*)))

// Defines block memory, allocator instance and a handle. On the example below, 
// the ALLOC_Allocator instance is myAllocatorObj and the handle is myAllocator.
// _name_ - the allocator name
// _size_ - fixed memory block size in bytes
// _objects_ - number of fixed memory blocks 
// e.g. ALLOC_DEFINE(myAllocator, 32, 10)
#define ALLOC_DEFINE(_name_, _size_, _objects_) \
    static char _name_##Memory[ALLOC_BLOCK_SIZE(_size_) * (_objects_)] = { 0 }; \
    static ALLOC_Allocator _name_##Obj = { #_name_, _name_##Memory, _size_, \
        ALLOC_BLOCK_SIZE(_size_), _objects_, NULL, 0, 0, 0, 0, 0 }; \
    static ALLOC_HANDLE _name_ = &_name_##Obj;

void ALLOC_Init(void);
void ALLOC_Term(void);
void* ALLOC_Alloc(ALLOC_HANDLE hAlloc, size_t size);
void* ALLOC_Calloc(ALLOC_HANDLE hAlloc, size_t num, size_t size);
void ALLOC_Free(ALLOC_HANDLE hAlloc, void* pBlock);

#ifdef __cplusplus
}
#endif

#endif  // _FB_ALLOCATOR_H




