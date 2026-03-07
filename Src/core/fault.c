#include "core/fault.h"
#include "core/uprint.h"

#include <string.h>

/*******************************************************************************
 * Private data
 ******************************************************************************/

static fault_t   fault_registry[FAULT_MAX_ENTRIES];
static uint32_t  fault_count   = 0;
static uint32_t  active_mask   = 0;   /* Bit N set while fault N != IDLE     */
static uint32_t  history_mask  = 0;   /* Bit N set if fault N ever triggered */

/*******************************************************************************
 * Private helpers
 ******************************************************************************/

static void mask_set_active(const fault_t *f)
{
    active_mask |= (1UL << f->index);
}

static void mask_clear_active(const fault_t *f)
{
    active_mask &= ~(1UL << f->index);
}

static void mask_set_history(const fault_t *f)
{
    history_mask |= (1UL << f->index);
}

/*******************************************************************************
 * Core API
 ******************************************************************************/

void fault_init(void)
{
    memset(fault_registry, 0, sizeof(fault_registry));
    fault_count  = 0;
    active_mask  = 0;
    history_mask = 0;
}

fault_handle_t fault_register(const fault_config_t *config)
{
    if(config == NULL)                   return NULL;
    if(config->detect == NULL)           return NULL;
    if(config->on_fault == NULL)         return NULL;
    if(fault_count >= FAULT_MAX_ENTRIES) return NULL;

    fault_t *f = &fault_registry[fault_count];

    f->cfg                = *config;
    f->state              = FAULT_STATE_IDLE;
    f->trigger_count      = 0;
    f->has_ever_triggered = false;
    f->index              = (uint8_t)fault_count;
    f->registered         = true;

    fault_count++;
    return f;
}

void fault_update(void)
{
    for(uint32_t i = 0; i < fault_count; i++)
    {
        fault_t *f = &fault_registry[i];

        if(!f->registered) continue;

        switch(f->state)
        {
            /* ----------------------------------------------------------------
             * IDLE — poll the detection function
             * ---------------------------------------------------------------- */
            case FAULT_STATE_IDLE:
            {
                if(f->cfg.detect())
                {
                    f->trigger_count++;
                    f->has_ever_triggered = true;

                    mask_set_active(f);
                    mask_set_history(f);

                    f->cfg.on_fault();

                    uprint("[FAULT] '%s' triggered (#%u)\r\n",
                           f->cfg.name, f->trigger_count);

                    if(f->cfg.recovery_ms > 0)
                    {
                        simple_timer_setup(&f->recovery_timer,
                                           f->cfg.recovery_ms, false);
                        f->state = FAULT_STATE_RECOVERING;
                    }
                    else
                    {
                        f->state = FAULT_STATE_ACTIVE;
                        /* stays ACTIVE — latched until fault_clear() */
                    }
                }
                break;
            }

            /* ----------------------------------------------------------------
             * ACTIVE — latched fault, wait for fault_clear()
             * ---------------------------------------------------------------- */
            case FAULT_STATE_ACTIVE:
            {
                /* Nothing to do — fault_clear() drives the transition */
                break;
            }

            /* ----------------------------------------------------------------
             * RECOVERING — waiting for the cooldown timer.
             *
             * When the timer elapses the condition is re-checked:
             *   still present → reset timer silently, stay RECOVERING
             *   gone          → call on_recover(), return to IDLE
             * ---------------------------------------------------------------- */
            case FAULT_STATE_RECOVERING:
            {
                if(simple_timer_has_elapsed(&f->recovery_timer))
                {
                    if(f->cfg.detect())
                    {
                        uprint("[FAULT] '%s' still active, extending cooldown\r\n",
                               f->cfg.name);
                        simple_timer_setup(&f->recovery_timer,
                                           f->cfg.recovery_ms, false);
                    }
                    else
                    {
                        uprint("[FAULT] '%s' recovering\r\n", f->cfg.name);

                        mask_clear_active(f);
                        if(f->cfg.on_recover != NULL)
                        {
                            f->cfg.on_recover();
                        }

                        /* history_mask bit intentionally kept set */

                        f->state = FAULT_STATE_IDLE;
                    }
                }
                break;
            }

            default: break;
        }
    }
}

/*******************************************************************************
 * Per-fault control
 ******************************************************************************/

void fault_clear(fault_handle_t handle)
{
    if(handle == NULL) return;

    fault_t *f = (fault_t*)handle;

    if(f->state != FAULT_STATE_ACTIVE) return;

    if(!f->cfg.detect())
    {
        uprint("[FAULT] '%s' cleared manually\r\n", f->cfg.name);

        if(f->cfg.on_recover != NULL)
        {
            f->cfg.on_recover();
        }

        mask_clear_active(f);
        /* history_mask bit intentionally kept set */

        f->state = FAULT_STATE_IDLE;
    }
    else
    {
        uprint("[FAULT] '%s' clear ignored — condition still active\r\n",
               f->cfg.name);
    }
}

fault_state_e fault_get_state(fault_handle_t handle)
{
    if(handle == NULL) return FAULT_STATE_IDLE;
    return ((fault_t*)handle)->state;
}

uint32_t fault_get_trigger_count(fault_handle_t handle)
{
    if(handle == NULL) return 0;
    return ((fault_t*)handle)->trigger_count;
}

bool fault_has_ever_triggered(fault_handle_t handle)
{
    if(handle == NULL) return false;
    return ((fault_t*)handle)->has_ever_triggered;
}

void fault_history_clear(fault_handle_t handle)
{
    if(handle == NULL) return;

    fault_t *f = (fault_t*)handle;
    f->has_ever_triggered = false;
    history_mask &= ~(1UL << f->index);
}

/*******************************************************************************
 * Diagnostic / protocol API
 ******************************************************************************/

fault_masks_t fault_get_masks(void)
{
    fault_masks_t m;
    m.active_mask  = active_mask;
    m.history_mask = history_mask;
    return m;
}

bool fault_any_active(void)
{
    return (active_mask != 0);
}

bool fault_any_in_history(void)
{
    return (history_mask != 0);
}

void fault_history_clear_all(void)
{
    for(uint32_t i = 0; i < fault_count; i++)
    {
        fault_registry[i].has_ever_triggered = false;
    }
    history_mask = 0;
}

void fault_print_status(void)
{
    static const char *state_names[] = {
        "IDLE",
        "ACTIVE",
        "RECOVERING",
    };

    uprint("===================================\r\n");
    uprint("FAULT STATUS (%u registered)\r\n", fault_count);
    uprint("active_mask  = 0x%x\r\n", active_mask);
    uprint("history_mask = 0x%x\r\n", history_mask);
    uprint("===================================\r\n");

    for(uint32_t i = 0; i < fault_count; i++)
    {
        fault_t *f = &fault_registry[i];
        uprint("  [%u] %s  state=%s  count=%u  history=%s\r\n",
               f->index,
               f->cfg.name,
               state_names[f->state],
               f->trigger_count,
               f->has_ever_triggered ? "YES" : "no");
    }

    uprint("===================================\r\n");
}