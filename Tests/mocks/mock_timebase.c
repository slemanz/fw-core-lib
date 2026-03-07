#include "mock_timebase.h"
#include "interface/interface.h"
#include "CppUTestExt/MockSupport_c.h"

static uint64_t s_ticks = 0u;

/* ================================================================== */
/*  Test control API                                                   */
/* ================================================================== */

void mock_timebase_set_ticks(uint64_t ticks) { s_ticks  = ticks; }
void mock_timebase_advance  (uint64_t delta) { s_ticks += delta; }
void mock_timebase_reset    (void)           { s_ticks  = 0u;   }

/* ================================================================== */
/*  timebase_get — replaces interface_timebase.c at link time         */
/* ================================================================== */

uint64_t timebase_get(void)
{
    mock_c()->actualCall("timebase_get");
    return s_ticks;
}

void timebase_deinit(void)
{
    /* Nothing to release in the stub */
}
