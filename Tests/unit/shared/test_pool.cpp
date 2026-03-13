#include "CppUTest/TestHarness.h"

extern "C"
{
#include "shared/pool.h"
#include <string.h>
}

/* ================================================================== */
/*  TEST GROUP: Pool (standard)                                        */
/* ================================================================== */

TEST_GROUP(Pool)
{
    void setup()
    {
        pool_Init();
    }

    void teardown() {}
};

/* ---- Allocation -------------------------------------------------- */

TEST(Pool, AllocateReturnsNonNull)
{
    CHECK(pool_Allocate() != NULL);
}

TEST(Pool, AllocateReturnsZeroedBlock)
{
    uint8_t *ptr = (uint8_t *)pool_Allocate();
    CHECK(ptr != NULL);

    for (uint32_t i = 0; i < POOL_BLOCK_SIZE; i++)
    {
        BYTES_EQUAL(0, ptr[i]);
    }
}

TEST(Pool, TwoAllocationsReturnDistinctPointers)
{
    void *a = pool_Allocate();
    void *b = pool_Allocate();

    CHECK(a != NULL);
    CHECK(b != NULL);
    CHECK(a != b);
}

TEST(Pool, AllocateAllBlocksSucceeds)
{
    for (uint32_t i = 0; i < POOL_BLOCK_COUNT; i++)
    {
        CHECK(pool_Allocate() != NULL);
    }
}

TEST(Pool, AllocateReturnsNullWhenExhausted)
{
    for (uint32_t i = 0; i < POOL_BLOCK_COUNT; i++)
        pool_Allocate();

    POINTERS_EQUAL(NULL, pool_Allocate());
}

/* ---- Free -------------------------------------------------------- */

TEST(Pool, FreeAndReallocateSucceeds)
{
    void *ptr = pool_Allocate();
    pool_Free(ptr);

    void *ptr2 = pool_Allocate();
    CHECK(ptr2 != NULL);
}

TEST(Pool, FreeNullDoesNotCrash)
{
    pool_Free(NULL);
}

TEST(Pool, FreePointerOutsidePoolIsIgnored)
{
    uint8_t stack_var;
    pool_Free(&stack_var);

    UNSIGNED_LONGS_EQUAL(POOL_BLOCK_COUNT, pool_GetFreeBlockCount());
}

TEST(Pool, ReallocatedBlockIsZeroed)
{
    uint8_t *ptr = (uint8_t *)pool_Allocate();
    memset(ptr, 0xAA, POOL_BLOCK_SIZE);
    pool_Free(ptr);

    uint8_t *ptr2 = (uint8_t *)pool_Allocate();
    for (uint32_t i = 0; i < POOL_BLOCK_SIZE; i++)
    {
        BYTES_EQUAL(0, ptr2[i]);
    }
}

/* ---- GetFreeBlockCount ------------------------------------------- */

TEST(Pool, FreeBlockCountStartsFull)
{
    UNSIGNED_LONGS_EQUAL(POOL_BLOCK_COUNT, pool_GetFreeBlockCount());
}

TEST(Pool, FreeBlockCountDecrementsOnAllocate)
{
    pool_Allocate();
    UNSIGNED_LONGS_EQUAL(POOL_BLOCK_COUNT - 1, pool_GetFreeBlockCount());
}

TEST(Pool, FreeBlockCountIncrementsOnFree)
{
    void *ptr = pool_Allocate();
    pool_Free(ptr);
    UNSIGNED_LONGS_EQUAL(POOL_BLOCK_COUNT, pool_GetFreeBlockCount());
}

/* ---- IsFromPool -------------------------------------------------- */

TEST(Pool, IsFromPoolReturnsTrueForAllocatedBlock)
{
    void *ptr = pool_Allocate();
    CHECK(pool_IsFromPool(ptr));
}

TEST(Pool, IsFromPoolReturnsFalseForStackPointer)
{
    uint8_t stack_var;
    CHECK_FALSE(pool_IsFromPool(&stack_var));
}

TEST(Pool, IsFromPoolReturnsFalseForNull)
{
    CHECK_FALSE(pool_IsFromPool(NULL));
}