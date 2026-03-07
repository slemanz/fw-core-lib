#ifndef INC_POOL_H_
#define INC_POOL_H_

#include <stdint.h>
#include <stddef.h>

/* ========== Configuration Parameters ========== */

#define POOL_BLOCK_COUNT            64  // Number of small blocks
#define POOL_BLOCK_SIZE             32  // Size of each small block in bytes

#define POOL_BIG_BLOCK_COUNT        16   // Number of Big blocks
#define POOL_BIG_BLOCK_SIZE         128 // Size of each big block in bytes

#define POOL_ALIGNAMENT             4   // Align Memory blocks to 4 bytes


/* ========== Data Structures ========== */

typedef struct MemBlock{
    struct MemBlock *next;
}MemBlock_t;

typedef struct
{
    uint8_t pool[POOL_BLOCK_COUNT][POOL_BLOCK_SIZE];
    MemBlock_t *freelist;
}MemPool_t;

typedef struct{
    uint8_t pool[POOL_BIG_BLOCK_COUNT][POOL_BIG_BLOCK_SIZE];
    MemBlock_t *freelist;
}MemPoolBig_t;

/* =========== Standard Pool =========== */

void pool_Init(void);
void *pool_Allocate(void);
void pool_Free(void *ptr);

uint32_t pool_GetFreeBlockCount(void);
uint8_t pool_IsFromPool(void *ptr);

/* ============= Big Pool ================ */

void poolBig_Init(void);
void *poolBig_Allocate(void);
void poolBig_Free(void *ptr);

uint32_t poolBig_GetFreeBlockCount(void);
uint8_t poolBig_IsFromPool(void *ptr);

#endif /* INC_POOL_H_ */