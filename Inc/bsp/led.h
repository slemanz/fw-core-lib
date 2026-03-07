#ifndef INC_LED_H_
#define INC_LED_H_

#include <stdint.h>
#include <stdbool.h>
#include "interface/interface.h"

/* ================================================================== */
/*  Single-colour LED                                                 */
/* ================================================================== */

typedef struct led_t* ledPtr_t;

/* -- creation / destruction ---------------------------------------- */

ledPtr_t    led_create        (const char *name, uint8_t pin_id);
ledPtr_t    led_createWithUuid(const char *name, uint8_t pin_id, uint32_t uuid);
ledPtr_t    led_getByUuid     (uint32_t uuid);
void        led_destroy       (ledPtr_t led);

/* -- configuration ------------------------------------------------- */

io_status_t led_invertLogic   (ledPtr_t led);

/* -- control ------------------------------------------------------- */

io_status_t led_turn_on       (ledPtr_t led);
io_status_t led_turn_off      (ledPtr_t led);
io_status_t led_toggle        (ledPtr_t led);

/* -- diagnostics --------------------------------------------------- */

void        led_displayInfo   (ledPtr_t led);
void        led_displayAll    (void);

/* ================================================================== */
/*  RGB LED                                                            */
/* ================================================================== */

typedef struct led_rgb_t *ledRgbPtr_t;

typedef enum
{
    LED_RGB_COLOR_OFF     = 0,
    LED_RGB_COLOR_RED,
    LED_RGB_COLOR_GREEN,
    LED_RGB_COLOR_BLUE,
    LED_RGB_COLOR_YELLOW,
    LED_RGB_COLOR_CYAN,
    LED_RGB_COLOR_MAGENTA,
    LED_RGB_COLOR_WHITE,
} led_rgb_color_e;

/* -- creation / destruction ---------------------------------------- */

ledRgbPtr_t led_rgb_create        (const char *name,
                                    uint8_t pin_r,
                                    uint8_t pin_g,
                                    uint8_t pin_b);

ledRgbPtr_t led_rgb_createWithUuid(const char *name,
                                    uint8_t pin_r,
                                    uint8_t pin_g,
                                    uint8_t pin_b,
                                    uint8_t uuid);

void        led_rgb_destroy       (ledRgbPtr_t led);
ledRgbPtr_t led_rgb_getByUuid     (uint8_t uuid);

/* -- configuration ------------------------------------------------- */

io_status_t led_rgb_invertLogic   (ledRgbPtr_t led);

/* -- control ------------------------------------------------------- */

io_status_t         led_rgb_set     (ledRgbPtr_t led, led_rgb_color_e color);
io_status_t         led_rgb_setRaw  (ledRgbPtr_t led, bool r, bool g, bool b);
io_status_t         led_rgb_off     (ledRgbPtr_t led);
io_status_t         led_rgb_toggle  (ledRgbPtr_t led, led_rgb_color_e color);
led_rgb_color_e     led_rgb_getColor(ledRgbPtr_t led);

/* -- diagnostics --------------------------------------------------- */

void        led_rgb_displayInfo   (ledRgbPtr_t led);
void        led_rgb_displayAll    (void);

#endif /* INC_LED_H_ */