// https://www.codeproject.com/Articles/1272619/A-Fixed-Block-Memory-Allocator-in-C
//
// The x_allocator is a fixed block memory allocator that handles multiple 
// block sizes by using two or more fb_allocator objects. Typically users 
// create a thin wrapper module for each x_allocator managed memory blocks. 
//
// For example, create a XAllocData instance and wrapper functions 
// in my_allocator.c:
//
// #define MAX_32_BLOCKS   10
// #define MAX_128_BLOCKS   5
// #define MAX_512_BLOCKS   2
//
// #define BLOCK_32_SIZE           32 + XALLOC_BLOCK_META_DATA_SIZE
// #define BLOCK_128_SIZE          128 + XALLOC_BLOCK_META_DATA_SIZE
// #define BLOCK_512_SIZE          512 + XALLOC_BLOCK_META_DATA_SIZE
//
// // Define each fb_allocator instance
// ALLOC_DEFINE(myAllocator32, BLOCK_32_SIZE, MAX_32_BLOCKS)
// ALLOC_DEFINE(myAllocator128, BLOCK_128_SIZE, MAX_128_BLOCKS)
// ALLOC_DEFINE(myAllocator512, BLOCK_512_SIZE, MAX_512_BLOCKS)
//
// // An array of allocators sorted by smallest to largest block 
// static ALLOC_Allocator* allocators[] = {
//    &myAllocator32Obj,
//    &myAllocator128Obj,
//    &myAllocator512Obj
// };
//
// #define MAX_ALLOCATORS   (sizeof(allocators) / sizeof(allocators[0]))
//
// static XAllocData self = { allocators, MAX_ALLOCATORS };
//
// // Thin allocator wrapper function implementations call XALLOC
// void* MYALLOC_Alloc(size_t size) { return XALLOC_Alloc(&self, size); }
// void MYALLOC_Free(void* ptr) { XALLOC_Free(ptr); }
// void* MYALLOC_Realloc(void *ptr, size_t new_size) { return XALLOC_Realloc(&self, ptr, new_size); }
// void* MYALLOC_Calloc(size_t num, size_t size) { return XALLOC_Calloc(&self, num, size); }
//
// Expose the allocator functions in my_allocator.h:
//
// void* MYALLOC_Alloc(size_t size);
// void MYALLOC_Free(void* ptr);
// void* MYALLOC_Realloc(void *ptr, size_t new_size);
// void* MYALLOC_Calloc(size_t num, size_t size);

#ifndef _X_ALLOCATOR_H
#define _X_ALLOCATOR_H

#include "fb_allocator.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Overhead bytes added to each XALLOC memory block
#define XALLOC_BLOCK_META_DATA_SIZE  sizeof(ALLOC_Allocator*)

typedef struct
{
    // Array of allocator instances sorted from smallest to largest block
    ALLOC_Allocator* const *allocators;

    // Number of allocator instances stored within the allocators array
    const UINT16 maxAllocators;
} XAllocData;

void* XALLOC_Alloc(XAllocData* self, size_t size);
void XALLOC_Free(void* ptr);
void* XALLOC_Realloc(XAllocData* self, void *ptr, size_t new_size);
void* XALLOC_Calloc(XAllocData* self, size_t num, size_t size);

#ifdef __cplusplus
}
#endif

#endif // _X_ALLOCATOR_H
