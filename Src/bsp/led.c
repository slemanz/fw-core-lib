#include "bsp/led.h"
#include "core/uprint.h"
#include "shared/pool.h"

/* ================================================================== */
/*  Internal structures                                                */
/* ================================================================== */

struct led_t
{
    const char *name;
    uint8_t     pin_id;
    uint32_t    uuid;
    bool        inverted;
    ledPtr_t    next;
};

struct led_rgb_t
{
    const char      *name;
    uint8_t          pin_r;
    uint8_t          pin_g;
    uint8_t          pin_b;
    uint8_t          uuid;
    bool             inverted;
    led_rgb_color_e  current_color;
    ledRgbPtr_t      next;
};

/* ================================================================== */
/*  Linked list state                                                  */
/* ================================================================== */

static ledPtr_t    s_led_head       = NULL;
static uint32_t    s_uuid_count     = 1u;

static ledRgbPtr_t s_led_rgb_head   = NULL;
static uint8_t     s_uuid_rgb_count = 1u;


/* ================================================================== */
/*  Forward declarations                                               */
/* ================================================================== */

static void led_list_insert       (ledPtr_t led);
static void led_list_delete       (ledPtr_t led);
static bool led_list_uuid_exists  (uint32_t uuid);

static void rgb_list_insert       (ledRgbPtr_t led);
static void rgb_list_delete       (ledRgbPtr_t led);
static bool rgb_list_uuid_exists  (uint8_t uuid);

/* ================================================================== */
/*  Single LED — creation                                             */
/* ================================================================== */

ledPtr_t led_create(const char *name, uint8_t pin_id)
{
    ledPtr_t led = (ledPtr_t)pool_Allocate();

    if (led == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    led->name     = name;
    led->pin_id   = pin_id;
    led->inverted = false;
    led->next     = NULL;

    while(led_list_uuid_exists(s_uuid_count))
    {
        s_uuid_count++;
    }
    led->uuid = s_uuid_count++;

    led_list_insert(led);
    uprint("*** %s created (UUID %lu) ***\r\n", led->name, led->uuid);

    return led;
}

ledPtr_t led_createWithUuid(const char *name, uint8_t pin_id, uint32_t uuid)
{
    if (led_list_uuid_exists(uuid))
    {
        uprint("Failed to create %s: UUID %lu already exists\r\n", name, uuid);
        return NULL;
    }

    ledPtr_t led = (ledPtr_t)pool_Allocate();

    if (led == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    led->name     = name;
    led->pin_id   = pin_id;
    led->uuid     = uuid;
    led->inverted = false;
    led->next     = NULL;

    led_list_insert(led);
    uprint("*** %s created with UUID %lu ***\r\n", led->name, led->uuid);

    return led;
}

void led_destroy(ledPtr_t led)
{
    if (led == NULL) return;
    uprint("*** %s destroyed ***\r\n", led->name);
    led_list_delete(led);
    pool_Free(led);
}

ledPtr_t led_getByUuid(uint32_t uuid)
{
    ledPtr_t cur = s_led_head;
    while (cur != NULL)
    {
        if (cur->uuid == uuid) return cur;
        cur = cur->next;
    }
    return NULL;
}

/* ================================================================== */
/*  Single LED — configuration                                        */
/* ================================================================== */

io_status_t led_invertLogic(ledPtr_t led)
{
    if (led == NULL) return IO_ERR_NULL;
    led->inverted = true;
    return IO_OK;
}

/* ================================================================== */
/*  Single LED — control                                              */
/* ================================================================== */

io_status_t led_turn_on(ledPtr_t led)
{
    if (led == NULL) return IO_ERR_NULL;
    return IO_write(led->pin_id, led->inverted ? IO_PIN_LOW : IO_PIN_HIGH);
}

io_status_t led_turn_off(ledPtr_t led)
{
    if (led == NULL) return IO_ERR_NULL;
    return IO_write(led->pin_id, led->inverted ? IO_PIN_HIGH : IO_PIN_LOW);
}

io_status_t led_toggle(ledPtr_t led)
{
    if (led == NULL) return IO_ERR_NULL;
    return IO_toggle(led->pin_id);
}


/* ================================================================== */
/*  Single LED — diagnostics                                          */
/* ================================================================== */

void led_displayInfo(ledPtr_t led)
{
    if (led == NULL) return;
    uprint("Device: %s | UUID: %lu | pin_id: %u\r\n",
           led->name, led->uuid, led->pin_id);
}

void led_displayAll(void)
{
    ledPtr_t cur = s_led_head;
    while (cur != NULL)
    {
        uprint("  %s (UUID: %lu, pin_id: %u)\r\n",
               cur->name, cur->uuid, cur->pin_id);
        cur = cur->next;
    }
}

/* ================================================================== */
/*  Single LED — list helpers                                         */
/* ================================================================== */

static void led_list_insert(ledPtr_t led)
{
    if (s_led_head == NULL) { s_led_head = led; return; }
    ledPtr_t cur = s_led_head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = led;
}

static void led_list_delete(ledPtr_t led)
{
    ledPtr_t cur  = s_led_head;
    ledPtr_t prev = NULL;
    while (cur != NULL)
    {
        if (cur == led)
        {
            if (prev == NULL) s_led_head = cur->next;
            else              prev->next = cur->next;
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}

static bool led_list_uuid_exists(uint32_t uuid)
{
    ledPtr_t cur = s_led_head;
    while (cur != NULL) { if (cur->uuid == uuid) return true; cur = cur->next; }
    return false;
}

/* ================================================================== */
/*  RGB LED — creation                                                */
/* ================================================================== */

ledRgbPtr_t led_rgb_create(const char *name,
                            uint8_t pin_r, uint8_t pin_g, uint8_t pin_b)
{
    ledRgbPtr_t led = (ledRgbPtr_t)pool_Allocate();

    if (led == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    led->name          = name;
    led->pin_r         = pin_r;
    led->pin_g         = pin_g;
    led->pin_b         = pin_b;
    led->inverted      = false;
    led->current_color = LED_RGB_COLOR_OFF;
    led->next          = NULL;

    while (rgb_list_uuid_exists(s_uuid_rgb_count))
        s_uuid_rgb_count++;
    led->uuid = s_uuid_rgb_count++;

    rgb_list_insert(led);
    uprint("*** %s created (UUID %u) ***\r\n", led->name, led->uuid);

    return led;
}

ledRgbPtr_t led_rgb_createWithUuid(const char *name,
                                    uint8_t pin_r, uint8_t pin_g, uint8_t pin_b,
                                    uint8_t uuid)
{
    if (rgb_list_uuid_exists(uuid))
    {
        uprint("Failed to create %s: UUID %u already exists\r\n", name, uuid);
        return NULL;
    }

    ledRgbPtr_t led = (ledRgbPtr_t)pool_Allocate();

    if (led == NULL)
    {
        uprint("Low memory, cannot create device\r\n");
        return NULL;
    }

    led->name          = name;
    led->pin_r         = pin_r;
    led->pin_g         = pin_g;
    led->pin_b         = pin_b;
    led->uuid          = uuid;
    led->inverted      = false;
    led->current_color = LED_RGB_COLOR_OFF;
    led->next          = NULL;

    rgb_list_insert(led);
    uprint("*** %s created with UUID %u ***\r\n", led->name, led->uuid);

    return led;
}

void led_rgb_destroy(ledRgbPtr_t led)
{
    if (led == NULL) return;
    uprint("*** %s destroyed ***\r\n", led->name);
    rgb_list_delete(led);
    pool_Free(led);
}

ledRgbPtr_t led_rgb_getByUuid(uint8_t uuid)
{
    ledRgbPtr_t cur = s_led_rgb_head;
    while (cur != NULL) { if (cur->uuid == uuid) return cur; cur = cur->next; }
    return NULL;
}

/* ================================================================== */
/*  RGB LED — configuration                                           */
/* ================================================================== */

io_status_t led_rgb_invertLogic(ledRgbPtr_t led)
{
    if (led == NULL) return IO_ERR_NULL;
    led->inverted = true;
    return IO_OK;
}

/* ================================================================== */
/*  RGB LED — control                                                 */
/* ================================================================== */

io_status_t led_rgb_set(ledRgbPtr_t led, led_rgb_color_e color)
{
    if (led == NULL) return IO_ERR_NULL;

    bool r = (color == LED_RGB_COLOR_RED     ||
              color == LED_RGB_COLOR_YELLOW  ||
              color == LED_RGB_COLOR_MAGENTA ||
              color == LED_RGB_COLOR_WHITE);

    bool g = (color == LED_RGB_COLOR_GREEN  ||
              color == LED_RGB_COLOR_YELLOW ||
              color == LED_RGB_COLOR_CYAN   ||
              color == LED_RGB_COLOR_WHITE);

    bool b = (color == LED_RGB_COLOR_BLUE    ||
              color == LED_RGB_COLOR_CYAN    ||
              color == LED_RGB_COLOR_MAGENTA ||
              color == LED_RGB_COLOR_WHITE);

    io_status_t s;

    s = IO_write(led->pin_r, (r ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    s = IO_write(led->pin_g, (g ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    s = IO_write(led->pin_b, (b ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    led->current_color = color;
    return IO_OK;
}

io_status_t led_rgb_setRaw(ledRgbPtr_t led, bool r, bool g, bool b)
{
    if (led == NULL) return IO_ERR_NULL;

    io_status_t s;

    s = IO_write(led->pin_r, (r ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    s = IO_write(led->pin_g, (g ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    s = IO_write(led->pin_b, (b ^ led->inverted) ? IO_PIN_HIGH : IO_PIN_LOW);
    if (s != IO_OK) return s;

    led->current_color = LED_RGB_COLOR_OFF;
    if ( r && !g && !b) led->current_color = LED_RGB_COLOR_RED;
    if (!r &&  g && !b) led->current_color = LED_RGB_COLOR_GREEN;
    if (!r && !g &&  b) led->current_color = LED_RGB_COLOR_BLUE;
    if ( r &&  g && !b) led->current_color = LED_RGB_COLOR_YELLOW;
    if (!r &&  g &&  b) led->current_color = LED_RGB_COLOR_CYAN;
    if ( r && !g &&  b) led->current_color = LED_RGB_COLOR_MAGENTA;
    if ( r &&  g &&  b) led->current_color = LED_RGB_COLOR_WHITE;

    return IO_OK;
}

io_status_t led_rgb_off(ledRgbPtr_t led)
{
    return led_rgb_set(led, LED_RGB_COLOR_OFF);
}

led_rgb_color_e led_rgb_getColor(ledRgbPtr_t led)
{
    if (led == NULL) return LED_RGB_COLOR_OFF;
    return led->current_color;
}

io_status_t led_rgb_toggle(ledRgbPtr_t led, led_rgb_color_e color)
{
    if (led == NULL) return IO_ERR_NULL;
    return (led->current_color == LED_RGB_COLOR_OFF)
        ? led_rgb_set(led, color)
        : led_rgb_set(led, LED_RGB_COLOR_OFF);
}

/* ================================================================== */
/*  RGB LED — diagnostics                                             */
/* ================================================================== */

void led_rgb_displayInfo(ledRgbPtr_t led)
{
    if (led == NULL) return;
    uprint("Device: %s | UUID: %u | pins R:%u G:%u B:%u\r\n",
           led->name, led->uuid, led->pin_r, led->pin_g, led->pin_b);
}

void led_rgb_displayAll(void)
{
    ledRgbPtr_t cur = s_led_rgb_head;
    while (cur != NULL)
    {
        uprint("  %s (UUID: %u)\r\n", cur->name, cur->uuid);
        cur = cur->next;
    }
}

/* ================================================================== */
/*  RGB LED — list helpers                                            */
/* ================================================================== */

static void rgb_list_insert(ledRgbPtr_t led)
{
    if (s_led_rgb_head == NULL) { s_led_rgb_head = led; return; }
    ledRgbPtr_t cur = s_led_rgb_head;
    while (cur->next != NULL) cur = cur->next;
    cur->next = led;
}

static void rgb_list_delete(ledRgbPtr_t led)
{
    ledRgbPtr_t cur  = s_led_rgb_head;
    ledRgbPtr_t prev = NULL;
    while (cur != NULL)
    {
        if (cur == led)
        {
            if (prev == NULL) s_led_rgb_head = cur->next;
            else              prev->next     = cur->next;
            return;
        }
        prev = cur;
        cur  = cur->next;
    }
}

static bool rgb_list_uuid_exists(uint8_t uuid)
{
    ledRgbPtr_t cur = s_led_rgb_head;
    while (cur != NULL) { if (cur->uuid == uuid) return true; cur = cur->next; }
    return false;
}