#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTestExt/MockSupport_c.h"

extern "C"
{
#include "core/simple-timer.h"
#include "mocks/mock_timebase.h"
}

/* ================================================================== */
/*  Helpers                                                            */
/* ================================================================== */

static simple_timer_t make_oneshot(uint64_t wait_ms)
{
    simple_timer_t t;
    simple_timer_setup(&t, wait_ms, false);
    return t;
}

static simple_timer_t make_autoreset(uint64_t wait_ms)
{
    simple_timer_t t;
    simple_timer_setup(&t, wait_ms, true);
    return t;
}


/* ================================================================== */
/*  Test Group                                                         */
/* ================================================================== */

TEST_GROUP(SimpleTimer)
{
    void setup()
    {
        mock_c()->ignoreOtherCalls();
        mock_timebase_reset();
    }

    void teardown()
    {
        mock_c()->checkExpectations();
        mock_c()->clear();
    }
};

/* -- One-shot behaviour ------------------------------------------- */

TEST(SimpleTimer, DoesNotElapseBforeDeadline)
{
    simple_timer_t t = make_oneshot(100);
    mock_timebase_advance(99);
    CHECK_FALSE(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, ElapsedExactlyAtDeadLine)
{
    simple_timer_t t = make_oneshot(100);
    mock_timebase_advance(100);
    CHECK(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, ElapsesWhenPastDeadline)
{
    simple_timer_t t = make_oneshot(100);
    mock_timebase_advance(200);
    CHECK(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, OneshotFiresOnlyOnce)
{
    simple_timer_t t = make_oneshot(100);
    mock_timebase_advance(100);
    CHECK      (simple_timer_has_elapsed(&t)); // first call
    CHECK_FALSE(simple_timer_has_elapsed(&t)); // second call
    CHECK_FALSE(simple_timer_has_elapsed(&t)); // third call
}

TEST(SimpleTimer, OneshotResetAllowsNewFire)
{
    simple_timer_t t = make_oneshot(100);
    mock_timebase_advance(100);
    CHECK(simple_timer_has_elapsed(&t));

    /* reset from current tick */
    simple_timer_reset(&t);
    CHECK_FALSE(simple_timer_has_elapsed(&t));

    mock_timebase_advance(100);
    CHECK(simple_timer_has_elapsed(&t));
}

/* -- Auto-reset behaviour ----------------------------------------- */

TEST(SimpleTimer, AutoResetFiresRepeatedly)
{
    simple_timer_t t = make_autoreset(50);
    mock_timebase_advance(50); CHECK(simple_timer_has_elapsed(&t));
    mock_timebase_advance(50); CHECK(simple_timer_has_elapsed(&t));
    mock_timebase_advance(50); CHECK(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, AutoResetDoesNotFireBetweenPeriods)
{
    simple_timer_t t = make_autoreset(50);
    mock_timebase_advance(50);
    CHECK(simple_timer_has_elapsed(&t));
    mock_timebase_advance(25);
    CHECK_FALSE(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, AutoResetCompensatesDrift)
{
    /*
     * Period = 100 ms, ideal deadlines at 100, 200, 300...
     * If we poll at 110 ms (10 ms late), the next deadline 
     * must remain exactly at 200 ms to stay accurate.
     */
    simple_timer_t t = make_autoreset(100);
    mock_timebase_advance(110); CHECK(simple_timer_has_elapsed(&t));
    mock_timebase_advance(80);  CHECK_FALSE(simple_timer_has_elapsed(&t));
    mock_timebase_advance(10);  CHECK(simple_timer_has_elapsed(&t));
}

/* -- Edge cases ---------------------------------------------------- */

TEST(SimpleTimer, ZeroWaitTimeElapsesImmediately)
{
    simple_timer_t t = make_oneshot(0);
    CHECK(simple_timer_has_elapsed(&t));
}

TEST(SimpleTimer, TimerStartedInTheFutureDoesNotElapse)
{
    mock_timebase_set_ticks(1000);
    simple_timer_t t;
    simple_timer_setup(&t, 500, false);

    mock_timebase_advance(499); CHECK_FALSE(simple_timer_has_elapsed(&t));
    mock_timebase_advance(1);   CHECK(simple_timer_has_elapsed(&t));
}

TEST_GROUP(SimpleTimerMock)
{
    void setup()
    {
        mock_timebase_reset();
    }

    void teardown()
    {
        mock_c()->checkExpectations();
        mock_c()->clear();
    }

    /* Declare N expected TIMEBASE_get() calls */
    void expect_ticks(int n)
    {
        for (int i = 0; i < n; i++)
            mock_c()->expectOneCall("timebase_get");
    }
};

TEST(SimpleTimerMock, SetupCallsTimebaseOnce)
{
    /*
     * simple_timer_setup() must read the clock exactly once to
     * record the start time (deadline = now + wait_ms).
     */
    expect_ticks(1);
    simple_timer_t t;
    simple_timer_setup(&t, 100u, false);
}

TEST(SimpleTimerMock, HasElapsedCallsTimebaseOnce)
{
    expect_ticks(2);
    simple_timer_t t;
    simple_timer_setup(&t, 100u, false);
    simple_timer_has_elapsed(&t);
}

TEST(SimpleTimerMock, ResetCallsTimebaseOnce)
{
    expect_ticks(1);   /* setup */
    mock_timebase_advance(100u);

    simple_timer_t t;
    simple_timer_setup(&t, 100u, false);

    expect_ticks(1);   /* first has_elapsed — fires */
    simple_timer_has_elapsed(&t);

    expect_ticks(1);   /* reset reads new start time */
    simple_timer_reset(&t);
}

TEST(SimpleTimerMock, AutoResetEachPeriodCallsTimebaseOnce)
{
    expect_ticks(1);   /* setup */
    simple_timer_t t;
    simple_timer_setup(&t, 50u, true);

    /* Period 1 */
    mock_timebase_advance(50u);
    expect_ticks(1);   /* fires + reloads */
    simple_timer_has_elapsed(&t);

    /* Period 2 */
    mock_timebase_advance(50u);
    expect_ticks(1);   /* fires + reloads */
    simple_timer_has_elapsed(&t);
}