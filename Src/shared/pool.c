    #include "shared/pool.h"
#include <string.h>

/* ============ Private Data ============ */

static MemPool_t memPool;
static MemPoolBig_t memPoolBig;

/* ========== Private Helper Functions ========== */

uint8_t pool_IsFromPool(void *ptr);
uint8_t poolBig_IsFromPool(void *ptr);

static void pool_InitGeneric(uint8_t *pool_start, uint32_t block_count, uint32_t block_size, MemBlock_t **freelist)
{
    *freelist = NULL;

    for(uint32_t i = 0; i < block_count; i++)
    {
        MemBlock_t *block = (MemBlock_t*)(pool_start + (i*block_size));
        block->next = *freelist;
        *freelist = block;
    }
}

static void *pool_AllocateGeneric(MemBlock_t **freelist, uint32_t block_size)
{
    if(*freelist == NULL)
    {
        return NULL;
    }

    MemBlock_t *block = *freelist;
    *freelist = block->next;

    memset(block, 0, block_size);
    
    return (void*)block;
}

static void pool_FreeGeneric(void *ptr, MemBlock_t **freelist, uint8_t (*is_from_pool)(void *))
{
    if(ptr == NULL)
    {
        return;
    }

    if(!is_from_pool(ptr))
    {
        return;
    }

    MemBlock_t *block = (MemBlock_t*)ptr;
    block->next = *freelist;
    *freelist = block;
}

static uint32_t pool_GetFreeBlockCountGeneric(MemBlock_t *freelist)
{
    uint32_t count = 0;
    MemBlock_t *current = freelist;

    while(current != NULL)
    {
        count++;
        current = current->next;
    }

    return count;
}

static uint8_t pool_IsFromPoolGeneric(void *ptr, uint8_t *pool_start, uint8_t *pool_end)
{
    if(ptr == NULL)
    {
        return 0;
    }

    uint8_t *bytePtr = (uint8_t*)ptr;

    if(bytePtr >= pool_start && bytePtr < pool_end)
    {
        return 1;
    }
    return 0;
}

/* ========== Standard Pool Implementation ========== */

void pool_Init(void)
{
    pool_InitGeneric((uint8_t*)&memPool.pool[0][0], POOL_BLOCK_COUNT, POOL_BLOCK_SIZE, &memPool.freelist);
}

void *pool_Allocate(void)
{
    return pool_AllocateGeneric(&memPool.freelist, POOL_BLOCK_SIZE);
}

void pool_Free(void *ptr)
{
    pool_FreeGeneric(ptr, &memPool.freelist, pool_IsFromPool);
}

uint32_t pool_GetFreeBlockCount(void)
{
    return pool_GetFreeBlockCountGeneric(memPool.freelist);
}

uint8_t pool_IsFromPool(void *ptr)
{
    uint8_t *pool_start = (uint8_t*)&memPool.pool[0][0];
    uint8_t *pool_end = (uint8_t*)&memPool.pool[POOL_BLOCK_COUNT - 1][POOL_BLOCK_SIZE];

    return pool_IsFromPoolGeneric(ptr, pool_start, pool_end);
}

/* ========== Big Pool Implementation ========== */

void poolBig_Init(void)
{
    pool_InitGeneric((uint8_t*)&memPoolBig.pool[0][0], POOL_BIG_BLOCK_COUNT, POOL_BIG_BLOCK_SIZE, &memPoolBig.freelist);
}

void *poolBig_Allocate(void)
{
    return pool_AllocateGeneric(&memPoolBig.freelist, POOL_BIG_BLOCK_SIZE);
}

void poolBig_Free(void *ptr)
{
    pool_FreeGeneric(ptr, &memPoolBig.freelist, poolBig_IsFromPool);
}

uint32_t poolBig_GetFreeBlockCount(void)
{
    return pool_GetFreeBlockCountGeneric(memPoolBig.freelist);
}

uint8_t poolBig_IsFromPool(void *ptr)
{
    uint8_t *pool_start = (uint8_t*)&memPoolBig.pool[0][0];
    uint8_t *pool_end = (uint8_t*)&memPoolBig.pool[POOL_BIG_BLOCK_COUNT - 1][POOL_BIG_BLOCK_SIZE];

    return pool_IsFromPoolGeneric(ptr, pool_start, pool_end);
}