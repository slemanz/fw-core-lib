#ifndef MOCK_TIMEBASE_H
#define MOCK_TIMEBASE_H

#include <stdint.h>
#include "interface/interface.h"

/** Set the simulated millisecond counter to an absolute value. */
void mock_timebase_set_ticks(uint64_t ticks);

/** Advance the simulated counter by delta milliseconds. */
void mock_timebase_advance  (uint64_t delta);

/** Reset the counter to zero. Call in setUp(). */
void mock_timebase_reset    (void);

#endif /* MOCK_TIMEBASE_H */
