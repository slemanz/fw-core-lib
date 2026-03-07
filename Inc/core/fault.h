#ifndef INC_FAULT_H
#define INC_FAULT_H

#include <stdint.h>
#include <stdbool.h>
#include "core/simple-timer.h"

#ifndef FAULT_MAX_ENTRIES
#define FAULT_MAX_ENTRIES   16 // the max musn't be more than 32
#endif /* FAULT_MAX_ENTRIES */

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef enum
{
    FAULT_STATE_IDLE       = 0, /* No fault present                           */
    FAULT_STATE_ACTIVE,         /* Fault latched, waiting for fault_clear()   */
    FAULT_STATE_RECOVERING,     /* Cooldown timer running                     */
} fault_state_e;

typedef bool (*fault_detect_fn_t)(void);
typedef void (*fault_action_fn_t)(void);
typedef void (*fault_recover_fn_t)(void);

typedef struct
{
    const char          *name;        /* Human-readable name (for debug)      */
    fault_detect_fn_t    detect;      /* Detection function   (required)      */
    fault_action_fn_t    on_fault;    /* Called on trigger    (required)      */
    fault_recover_fn_t   on_recover;  /* Called on recovery   (optional)      */
    uint32_t             recovery_ms; /* Cooldown in ms. 0 = latched.         */
} fault_config_t;

typedef struct
{
    fault_config_t  cfg;
    fault_state_e   state;
    simple_timer_t  recovery_timer;
    uint32_t        trigger_count;        /* How many times this fault fired  */
    bool            has_ever_triggered;   /* Set on first trigger, survives   */
                                          /* recovery — cleared only manually */
    uint8_t         index;                /* Bit position in the global masks */
    bool            registered;           /* Slot is in use                   */
} fault_t;

typedef fault_t *fault_handle_t;


/**
 * @brief  Pair of bitmasks returned by fault_get_masks().
 *
 *  active_mask   Bit N is set while fault N is ACTIVE or RECOVERING.
 *  history_mask  Bit N is set if fault N has ever triggered since the last
 *                fault_history_clear() / fault_history_clear_all().
 */
typedef struct
{
    uint32_t active_mask;
    uint32_t history_mask;
} fault_masks_t;

/*******************************************************************************
 * Core API
 ******************************************************************************/

void           fault_init(void);
fault_handle_t fault_register(const fault_config_t *config);
void           fault_update(void);

/*******************************************************************************
 * Per-fault control
 ******************************************************************************/

void           fault_clear(fault_handle_t handle);

fault_state_e  fault_get_state(fault_handle_t handle);
uint32_t       fault_get_trigger_count(fault_handle_t handle);

/** True if this fault has triggered at least once since last history clear. */
bool           fault_has_ever_triggered(fault_handle_t handle);

/** Clear the history flag for a single fault (does NOT affect active state). */
void           fault_history_clear(fault_handle_t handle);

/*******************************************************************************
 * Diagnostic / protocol API
 ******************************************************************************/

fault_masks_t  fault_get_masks(void);

/** True if ANY fault is currently ACTIVE or RECOVERING. */
bool           fault_any_active(void);

/** True if ANY fault has ever triggered (history not yet cleared). */
bool           fault_any_in_history(void);

/** Clear history flags for ALL faults at once. */
void           fault_history_clear_all(void);

/** Print a full status table via uprint (use as a CLI command handler). */
void           fault_print_status(void);


#endif /* INC_FAULT_H */