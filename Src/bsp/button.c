#include "bsp/button.h"
#include "core/uprint.h"
#include "core/simple-timer.h"
#include "shared/pool.h"
#include "shared/slist.h"

/* ================================================================== */
/*  Internal types                                                     */
/* ================================================================== */

typedef enum
{
    BTN_STATE_IDLE       = 0, /* Pin inactive (not pressed)                  */
    BTN_STATE_DEBOUNCE,       /* Pin went active — waiting for debounce window */
    BTN_STATE_PRESSED,        /* Debounced press confirmed                    */
    BTN_STATE_HELD,           /* Hold timer elapsed while still pressed       */
    BTN_STATE_RELEASE_DB,     /* Pin went inactive — waiting for release debounce */
} btn_state_e;

struct button_t
{
    const char     *name;
    uint8_t         pin_id;
    uint32_t        uuid;
    bool            inverted;

    uint32_t        debounce_ms;
    uint32_t        hold_time_ms;

    btn_state_e     state;
    simple_timer_t  debounce_timer;
    simple_timer_t  hold_timer;

    button_event_e  pending_event;
    bool            is_pressed;

    slist_node_t    node;
};

/* ================================================================== */
/*  List state                                                         */
/* ================================================================== */

static slist_head_t s_btn_list;
static uint32_t     s_uuid_count  = 1u;

/* ================================================================== */
/*  Forward declarations                                               */
/* ================================================================== */

static bool btn_uuid_exists(uint32_t uuid);

/* ================================================================== */
/*  Private helper                                                     */
/* ================================================================== */

/**
 * @brief Read the logical (debounce-corrected) pin state.
 *
 * Calls IO_read() and applies the inverted flag so the rest of the
 * state machine always works with an active-high convention.
 */
static bool read_pin_active(buttonPtr_t btn)
{
    uint8_t raw = IO_PIN_LOW;
    IO_read(btn->pin_id, &raw);
    return btn->inverted ? (raw == IO_PIN_LOW) : (raw == IO_PIN_HIGH);
}

/* ================================================================== */
/*  Creation                                                           */
/* ================================================================== */

buttonPtr_t button_create(const char *name,
                           uint8_t    pin_id,
                           uint32_t   debounce_ms,
                           uint32_t   hold_time_ms)
{
    buttonPtr_t btn = (buttonPtr_t)poolBig_Allocate();

    if (btn == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    btn->name          = name;
    btn->pin_id        = pin_id;
    btn->inverted      = false;
    btn->debounce_ms   = debounce_ms;
    btn->hold_time_ms  = hold_time_ms;
    btn->state         = BTN_STATE_IDLE;
    btn->pending_event = BUTTON_EVENT_NONE;
    btn->is_pressed    = false;

    while (btn_uuid_exists(s_uuid_count))
        s_uuid_count++;
    btn->uuid = s_uuid_count++;

    slist_push_front(&s_btn_list, &btn->node);
    uprint("*** %s created ***\r\n", btn->name);

    return btn;
}

buttonPtr_t button_createWithUuid(const char *name,
                                   uint8_t    pin_id,
                                   uint32_t   debounce_ms,
                                   uint32_t   hold_time_ms,
                                   uint32_t   uuid)
{
    if (btn_uuid_exists(uuid))
    {
        uprint("Failed to create %s: UUID %lu already exists\r\n", name, uuid);
        return NULL;
    }

    buttonPtr_t btn = (buttonPtr_t)poolBig_Allocate();

    if (btn == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    btn->name          = name;
    btn->pin_id        = pin_id;
    btn->uuid          = uuid;
    btn->inverted      = false;
    btn->debounce_ms   = debounce_ms;
    btn->hold_time_ms  = hold_time_ms;
    btn->state         = BTN_STATE_IDLE;
    btn->pending_event = BUTTON_EVENT_NONE;
    btn->is_pressed    = false;

    slist_push_front(&s_btn_list, &btn->node);
    uprint("*** %s created with UUID %lu ***\r\n", btn->name, btn->uuid);

    return btn;
}

void button_destroy(buttonPtr_t btn)
{
    if (btn == NULL) return;
    uprint("*** %s destroyed ***\r\n", btn->name);
    slist_remove(&s_btn_list, &btn->node);
    poolBig_Free(btn);
}

buttonPtr_t button_getByUuid(uint32_t uuid)
{
    slist_node_t *it;
    slist_for_each(it, &s_btn_list)
    {
        buttonPtr_t btn = container_of(it, struct button_t, node);
        if (btn->uuid == uuid) return btn;
    }
    return NULL;
}

/* ================================================================== */
/*  Configuration                                                      */
/* ================================================================== */

void button_invertLogic(buttonPtr_t btn)
{
    if (btn == NULL) return;
    btn->inverted = true;
}

/* ================================================================== */
/*  Update / events                                                    */
/* ================================================================== */

void button_update(buttonPtr_t btn)
{
    if (btn == NULL) return;

    bool active = read_pin_active(btn);

    switch (btn->state)
    {
        case BTN_STATE_IDLE:
        {
            if (active)
            {
                simple_timer_setup(&btn->debounce_timer, btn->debounce_ms, false);
                btn->state = BTN_STATE_DEBOUNCE;
            }
            break;
        }

        case BTN_STATE_DEBOUNCE:
        {
            if (!active)
            {
                /* Glitch — bail out before debounce window closes */
                btn->state = BTN_STATE_IDLE;
            }
            else if (simple_timer_has_elapsed(&btn->debounce_timer))
            {
                btn->state         = BTN_STATE_PRESSED;
                btn->is_pressed    = true;
                btn->pending_event = BUTTON_EVENT_PRESSED;

                if (btn->hold_time_ms > 0u)
                    simple_timer_setup(&btn->hold_timer, btn->hold_time_ms, false);
            }
            break;
        }

        case BTN_STATE_PRESSED:
        {
            if (!active)
            {
                simple_timer_setup(&btn->debounce_timer, btn->debounce_ms, false);
                btn->state = BTN_STATE_RELEASE_DB;
            }
            else if (btn->hold_time_ms > 0u &&
                     simple_timer_has_elapsed(&btn->hold_timer))
            {
                btn->state         = BTN_STATE_HELD;
                btn->pending_event = BUTTON_EVENT_HELD;
            }
            break;
        }

        case BTN_STATE_HELD:
        {
            if (!active)
            {
                simple_timer_setup(&btn->debounce_timer, btn->debounce_ms, false);
                btn->state = BTN_STATE_RELEASE_DB;
            }
            break;
        }

        case BTN_STATE_RELEASE_DB:
        {
            if (active)
            {
                /* Glitch on release — go back to pressed */
                btn->state = BTN_STATE_PRESSED;
            }
            else if (simple_timer_has_elapsed(&btn->debounce_timer))
            {
                btn->state         = BTN_STATE_IDLE;
                btn->is_pressed    = false;
                btn->pending_event = BUTTON_EVENT_RELEASED;
            }
            break;
        }

        default: break;
    }
}

button_event_e button_getEvent(buttonPtr_t btn)
{
    if (btn == NULL) return BUTTON_EVENT_NONE;

    button_event_e ev  = btn->pending_event;
    btn->pending_event = BUTTON_EVENT_NONE;
    return ev;
}

bool button_isPressed(buttonPtr_t btn)
{
    if (btn == NULL) return false;
    return btn->is_pressed;
}

/* ================================================================== */
/*  Diagnostics                                                        */
/* ================================================================== */

void button_displayInfo(buttonPtr_t btn)
{
    if (btn == NULL) return;
    uprint("Device: %s | UUID: %lu | pin_id: %u\r\n",
           btn->name, btn->uuid, btn->pin_id);
}

void button_displayAll(void)
{
    slist_node_t *it;
    slist_for_each(it, &s_btn_list)
    {
        buttonPtr_t btn = container_of(it, struct button_t, node);
        uprint("  %s (UUID: %lu)\r\n", btn->name, btn->uuid);
    }
}

/* ================================================================== */
/*  List helpers                                                       */
/* ================================================================== */

static bool btn_uuid_exists(uint32_t uuid)
{
    slist_node_t *it;
    slist_for_each(it, &s_btn_list)
    {
        buttonPtr_t btn = container_of(it, struct button_t, node);
        if (btn->uuid == uuid) return true;
    }
    return false;
}