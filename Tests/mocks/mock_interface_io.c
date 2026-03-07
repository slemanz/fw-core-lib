#include "mock_interface_io.h"
#include "interface/interface.h"
#include "CppUTestExt/MockSupport_c.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Pin level table (for IO_read out-parameter)                        */
/* ------------------------------------------------------------------ */

#define MAX_PINS 16u

static uint8_t s_pin_levels[MAX_PINS];

void mock_io_set_pin_level(uint8_t pin_id, uint8_t level)
{
    if (pin_id < MAX_PINS)
        s_pin_levels[pin_id] = level;
}

void mock_io_reset_levels(void)
{
    memset(s_pin_levels, IO_PIN_LOW, sizeof(s_pin_levels));
}

/* ================================================================== */
/*  Fake implementations                                               */
/* ================================================================== */

io_status_t IO_init(uint8_t pin_id)
{
    return (io_status_t)
        mock_c()->actualCall("IO_init")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->returnIntValueOrDefault(IO_OK);
}

io_status_t IO_write(uint8_t pin_id, uint8_t value)
{
    return (io_status_t)
        mock_c()->actualCall("IO_write")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->withUnsignedIntParameters("value",  value)
                ->returnIntValueOrDefault(IO_OK);
}

io_status_t IO_read(uint8_t pin_id, uint8_t *out_value)
{
    io_status_t ret = (io_status_t)
        mock_c()->actualCall("IO_read")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->returnIntValueOrDefault(IO_OK);

    if (out_value && pin_id < MAX_PINS)
        *out_value = s_pin_levels[pin_id];

    return ret;
}

io_status_t IO_toggle(uint8_t pin_id)
{
    return (io_status_t)
        mock_c()->actualCall("IO_toggle")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->returnIntValueOrDefault(IO_OK);
}

io_status_t IO_set_mode(uint8_t pin_id, uint8_t mode)
{
    return (io_status_t)
        mock_c()->actualCall("IO_set_mode")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->withUnsignedIntParameters("mode",   mode)
                ->returnIntValueOrDefault(IO_OK);
}

io_status_t IO_configure(uint8_t pin_id, uint8_t option, uint8_t value)
{
    return (io_status_t)
        mock_c()->actualCall("IO_configure")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->withUnsignedIntParameters("option", option)
                ->withUnsignedIntParameters("value",  value)
                ->returnIntValueOrDefault(IO_OK);
}

io_status_t IO_deinit(uint8_t pin_id)
{
    return (io_status_t)
        mock_c()->actualCall("IO_deinit")
                ->withUnsignedIntParameters("pin_id", pin_id)
                ->returnIntValueOrDefault(IO_OK);
}