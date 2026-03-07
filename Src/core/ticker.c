#include "core/ticker.h"
#include "core/uprint.h"

/*******************************************************************************
 * Private types
 ******************************************************************************/

/** Internal runtime state for each registered task. */
typedef struct
{
    const ticker_task_t *desc;   /**< Points back to the user's descriptor  */
    simple_timer_t       timer;  /**< One timer per task, armed at init      */
} ticker_entry_t;

/*******************************************************************************
 * Private state
 ******************************************************************************/

static ticker_entry_t _entries[TICKER_MAX_TASKS];
static uint32_t       _count = 0;

/*******************************************************************************
 * API
 ******************************************************************************/

void ticker_init(const ticker_task_t *tasks, uint32_t count)
{
    if(tasks == NULL || count == 0u) return;

    if(count > TICKER_MAX_TASKS)
    {
        uprint("[TICKER] Warning: %u tasks requested, only %u supported\r\n",
               count, TICKER_MAX_TASKS);
        count = TICKER_MAX_TASKS;
    }

    _count = count;

    for(uint32_t i = 0; i < _count; i++)
    {
        _entries[i].desc = &tasks[i];
        simple_timer_setup(&_entries[i].timer, tasks[i].period_ms, true);
    }

    uprint("[TICKER] Init OK - %u task(s) registered\r\n", _count);
}

void ticker_update(void)
{
    for(uint32_t i = 0; i < _count; i++)
    {
        ticker_entry_t *e = &_entries[i];

        if(simple_timer_has_elapsed(&e->timer))
        {
            e->desc->fn();
        }
    }
}

void ticker_print_tasks(void)
{
    uprint("===================================\r\n");
    uprint("TICKER - %u task(s)\r\n", _count);
    for(uint32_t i = 0; i < _count; i++)
    {
        uprint("  [%u] %s  %u ms\r\n",
               i,
               _entries[i].desc->name,
               _entries[i].desc->period_ms);
    }
    uprint("===================================\r\n");
}