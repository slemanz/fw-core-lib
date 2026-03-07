#ifndef INC_TICKER_H
#define INC_TICKER_H

#include <stdint.h>
#include "core/simple-timer.h"

/** Maximum number of tasks the scheduler can hold. Increase if needed. */
#ifndef TICKER_MAX_TASKS
#define TICKER_MAX_TASKS    16u
#endif

/*******************************************************************************
 * Public types
 ******************************************************************************/

/** Callback signature for a periodic task. */
typedef void (*ticker_fn_t)(void);

typedef struct
{
    ticker_fn_t  fn;         /**< Function to call periodically            */
    uint32_t     period_ms;  /**< Desired period in milliseconds           */
    const char  *name;       /**< Human-readable label (for diagnostics)   */
} ticker_task_t;

/*******************************************************************************
 * Convenience macros
 ******************************************************************************/

#define TICKER_TASK(function, period)  \
    { .fn = (function), .period_ms = (period), .name = #function }

#define TICKER_TASK_COUNT(array)  (sizeof(array) / sizeof((array)[0]))

void ticker_init(const ticker_task_t *tasks, uint32_t count);

void ticker_update(void);
void ticker_print_tasks(void);

#endif /* TICKER_H */