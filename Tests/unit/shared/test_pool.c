#include "unity_fixture.h"
#include "shared/pool.h"

/* ================================================================== */
/*  Test Group — Standard Pool                                        */
/* ================================================================== */

TEST_GROUP(Pool);

TEST_SETUP(Pool)
{
    pool_Init();
}

TEST_TEAR_DOWN(Pool)
{
    /* Nothing to clean up — pool_Init() in setUp resets everything */
}

/* ------------------------------------------------------------------ */
/*  Allocation                                                        */
/* ------------------------------------------------------------------ */

TEST(Pool, AllocateReturnsNonNull)
{
    void *ptr = pool_Allocate();
    TEST_ASSERT_NOT_NULL(ptr);
}

TEST(Pool, AllocateReturnsZeroedBlock)
{
    uint8_t *ptr = (uint8_t*)pool_Allocate();
    TEST_ASSERT_NOT_NULL(ptr);

    for(uint32_t i = 0; i < POOL_BLOCK_SIZE; i++)
    {
        TEST_ASSERT_EQUAL_UINT8(0, ptr[i]);
    }
}

TEST(Pool, TwoAllocationsReturnDistinctPointers)
{
    void *a = pool_Allocate();
    void *b = pool_Allocate();

    TEST_ASSERT_NOT_NULL(a);
    TEST_ASSERT_NOT_NULL(b);
    TEST_ASSERT_NOT_EQUAL(a, b);
}

TEST(Pool, AllocateAllBlocksSucceeds)
{
    void *ptrs[POOL_BLOCK_COUNT];

    for(uint32_t i = 0; i < POOL_BLOCK_COUNT; i++)
    {
        ptrs[i] = pool_Allocate();
        TEST_ASSERT_NOT_NULL_MESSAGE(ptrs[i], "Allocation failed before pool was exhausted");
    }
}

TEST(Pool, AllocateReturnsNullWhenExhausted)
{
    /* Drain the entire pool */
    for (uint32_t i = 0; i < POOL_BLOCK_COUNT; i++) pool_Allocate();

    TEST_ASSERT_NULL(pool_Allocate());
}

/* ================================================================== */
/*  Test runners                                                       */
/* ================================================================== */

TEST_GROUP_RUNNER(Pool)
{
    /* Allocation */
    RUN_TEST_CASE(Pool, AllocateReturnsNonNull);
    RUN_TEST_CASE(Pool, AllocateReturnsZeroedBlock);
    RUN_TEST_CASE(Pool, TwoAllocationsReturnDistinctPointers);
    RUN_TEST_CASE(Pool, AllocateAllBlocksSucceeds);
    RUN_TEST_CASE(Pool, AllocateReturnsNullWhenExhausted);
}