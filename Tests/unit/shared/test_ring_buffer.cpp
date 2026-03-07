#include "CppUTest/TestHarness.h"

extern "C"
{
#include "shared/ring-buffer.h"
}
/*
 * ring_buffer_setup() requires that size is a power of two because the
 * implementation uses a bitmask. All test buffers follow this rule.
 */

#define BUF_SIZE    8   /* power of two */

TEST_GROUP(RingBuffer)
{
    ring_buffer_t rb;
    uint8_t       backing[BUF_SIZE];

    void setup()
    {
        ring_buffer_setup(&rb, backing, BUF_SIZE);
    }

    void teardown() {}
};

/* ================================================================== */
/*  Tests — initial state                                             */
/* ================================================================== */

TEST(RingBuffer, IsEmptyAfterSetup)
{
    CHECK(ring_buffer_empty(&rb));
}

TEST(RingBuffer, ReadFromEmptyReturnsFalse)
{
    uint8_t byte;
    CHECK_FALSE(ring_buffer_read(&rb, &byte));
}

/* ================================================================== */
/*  Tests — write / read                                              */
/* ================================================================== */

TEST(RingBuffer, ReadReturnsTrueWhenNotFull)
{
    CHECK(ring_buffer_write(&rb, 0xABu));
}

TEST(RingBuffer, ReadReturnsByteWritten)
{
    ring_buffer_write(&rb, 0x42);

    uint8_t byte = 0;
    CHECK(ring_buffer_read(&rb, &byte));
    BYTES_EQUAL(0x42u, byte);
}

TEST(RingBuffer, NotEmptyAfterWrite)
{
    ring_buffer_write(&rb, 0x01);
    CHECK_FALSE(ring_buffer_empty(&rb));
}

TEST(RingBuffer, EmptyAfterWriteThenRead)
{
    uint8_t byte;
    ring_buffer_write(&rb, 0x55);
    ring_buffer_read(&rb, &byte);
    CHECK(ring_buffer_empty(&rb));
}


TEST(RingBuffer, PreservesFIFOOrder)
{
    /*
     * Write bytes 1, 2, 3 and verify they come back in the same order
     */
    ring_buffer_write(&rb, 1);
    ring_buffer_write(&rb, 2);
    ring_buffer_write(&rb, 3);

    uint8_t a, b, c;
    ring_buffer_read(&rb, &a);
    ring_buffer_read(&rb, &b);
    ring_buffer_read(&rb, &c);

    BYTES_EQUAL(1u, a);
    BYTES_EQUAL(2u, b);
    BYTES_EQUAL(3u, c);
}

/* ================================================================== */
/*  Tests — capacity                                                  */
/* ================================================================== */

TEST(RingBuffer, FillsToCapacityMinusOne)
{
    /*
     * The implementation sacrifices one slot to distinguish full from
     * empty, so a buffer of size N holds N-1 bytes.
     */
    uint8_t capacity = BUF_SIZE - 1;

    for(uint8_t i = 0; i < capacity; i++)
    {
        CHECK(ring_buffer_write(&rb, i));
    }
}

TEST(RingBuffer, WriteReturnsFalseWhenFull)
{
    uint8_t capacity = BUF_SIZE - 1;

    for(uint8_t i = 0; i < capacity; i++)
    {
        ring_buffer_write(&rb, i);
    }
    /* One more write must be rejected */
    CHECK_FALSE(ring_buffer_write(&rb, 0xFFu));
}


TEST(RingBuffer, OverflowDoesNotCorruptData)
{
    /*
     * Fill the buffer, reject the overflow write, then read back all
     * bytes and verify none were corrupted.
     */
    uint8_t capacity = BUF_SIZE - 1;

    for(uint8_t i = 0; i < capacity; i++)
    {
        ring_buffer_write(&rb, i);
    }

    ring_buffer_write(&rb, 0xFF); /* Rejected */

    for(uint8_t i = 0; i < capacity; i++)
    {
        uint8_t byte;
        ring_buffer_read(&rb, &byte);
        BYTES_EQUAL(i, byte);
    }
}

/* ================================================================== */
/*  Tests — wrap-around                                               */
/* ================================================================== */

TEST(RingBuffer, HandlesWrapAround)
{
    /*
     * Fill half the buffer, drain it, fill again — this forces the
     * write and read indices to wrap around the backing array.
     */
    uint8_t half = BUF_SIZE/2;
    uint8_t byte;

    // first half
    for(uint8_t i = 0; i < half; i++) ring_buffer_write(&rb, i);
    for(uint8_t i = 0; i < half; i++) ring_buffer_read(&rb, &byte);

    // second half
    for(uint8_t i = 0; i < half; i++) ring_buffer_write(&rb, (uint8_t)(i + 0x10));


    for (uint8_t i = 0; i < half; i++)
    {
        ring_buffer_read(&rb, &byte);
        BYTES_EQUAL((uint8_t)(i + 0x10u), byte);
    }
}

TEST(RingBuffer, EmptyAfterFullCycleWithWrapAround)
{
    uint8_t capacity = BUF_SIZE - 1;
    uint8_t byte;

    for (uint8_t i = 0; i < capacity; i++) ring_buffer_write(&rb, i);
    for (uint8_t i = 0; i < capacity; i++) ring_buffer_read(&rb, &byte);

    CHECK(ring_buffer_empty(&rb));
}