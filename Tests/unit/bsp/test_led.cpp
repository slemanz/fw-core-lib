#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include "CppUTestExt/MockSupport_c.h"

extern "C"
{
#include "bsp/led.h"
#include "shared/pool.h"
}

/* ------------------------------------------------------------------ */
/*  Pin IDs used across all tests                                     */
/* ------------------------------------------------------------------ */

static const uint8_t PIN_A = 0;   /* single LED          */
static const uint8_t PIN_R = 1;   /* RGB — red channel   */
static const uint8_t PIN_G = 2;   /* RGB — green channel */
static const uint8_t PIN_B = 3;   /* RGB — blue channel  */

/* ------------------------------------------------------------------ */
/*  Helper: set one IO_write expectation                               */
/* ------------------------------------------------------------------ */

static void expect_write(uint8_t pin_id, uint8_t value)
{
    mock_c()->expectOneCall("IO_write")
             ->withUnsignedIntParameters("pin_id", pin_id)
             ->withUnsignedIntParameters("value",  value);
}

/* ================================================================== */
/*  TEST GROUP: Led (single-colour)                                    */
/* ================================================================== */

TEST_GROUP(Led)
{
    ledPtr_t led;

    void setup()
    {
        pool_Init();
        led = NULL;
    }

    void teardown()
    {
        mock_c()->checkExpectations();
        mock_c()->clear();

        if(led)
        {
            led_destroy(led);
            led = NULL;
        }
    }
};

/* ---- creation / lookup ------------------------------------------- */

TEST(Led, CreateReturnsNonNull)
{
    led = led_create("L", PIN_A);
    CHECK(led != NULL);
}

TEST(Led, CreateWithUuidReturnsNonNull)
{
    led = led_createWithUuid("L", PIN_A, 42);
    CHECK(led != NULL);
}

TEST(Led, CreateWithDuplicateUuidReturnsNull)
{
    led = led_createWithUuid("L1", PIN_A, 10);
    CHECK(led != NULL);

    ledPtr_t dup = led_createWithUuid("L2", PIN_A, 10);
    CHECK(dup == NULL);
}

TEST(Led, GetByUuidFindsLed)
{
    led = led_createWithUuid("L", PIN_A, 77);
    POINTERS_EQUAL(led, led_getByUuid(77));
}

TEST(Led, GetByUuidReturnsNullForUnknown)
{
    CHECK(led_getByUuid(0xFFFF) == NULL);
}

/* ---- control ----------------------------------------------------- */

TEST(Led, TurnOn_WritesHigh)
{
    led = led_create("L", PIN_A);
    expect_write(PIN_A, IO_PIN_HIGH);

    LONGS_EQUAL(IO_OK, led_turn_on(led));
}

TEST(Led, TurnOff_WritesLow)
{
    led = led_create("L", PIN_A);
    expect_write(PIN_A, IO_PIN_LOW);

    LONGS_EQUAL(IO_OK, led_turn_off(led));
}

TEST(Led, Toggle_CallsIOToggle)
{
    led = led_create("L", PIN_A);
    mock_c()->expectOneCall("IO_toggle")
             ->withUnsignedIntParameters("pin_id", PIN_A);

    LONGS_EQUAL(IO_OK, led_toggle(led));
}

TEST(Led, Toggle_CalledTwice_ExpectsTwoCalls)
{
    led = led_create("L", PIN_A);
    mock_c()->expectOneCall("IO_toggle")->withUnsignedIntParameters("pin_id", PIN_A);
    mock_c()->expectOneCall("IO_toggle")->withUnsignedIntParameters("pin_id", PIN_A);

    led_toggle(led);
    led_toggle(led);
}

TEST(Led, TurnOn_NullLed_ReturnsError)
{
    LONGS_EQUAL(IO_ERR_NULL, led_turn_on(NULL));
}

TEST(Led, TurnOff_NullLed_ReturnsError)
{
    LONGS_EQUAL(IO_ERR_NULL, led_turn_off(NULL));
}

TEST(Led, Toggle_NullLed_ReturnsError)
{
    LONGS_EQUAL(IO_ERR_NULL, led_toggle(NULL));
}

/* ---- inverted logic --------------------------------------------- */

TEST(Led, Inverted_TurnOn_WritesLow)
{
    led = led_create("L", PIN_A);
    led_invertLogic(led);
    expect_write(PIN_A, IO_PIN_LOW);

    LONGS_EQUAL(IO_OK, led_turn_on(led));
}

TEST(Led, Inverted_TurnOff_WritesHigh)
{
    led = led_create("L", PIN_A);
    led_invertLogic(led);
    expect_write(PIN_A, IO_PIN_HIGH);

    LONGS_EQUAL(IO_OK, led_turn_off(led));
}

/* ---- driver error propagation ----------------------------------- */

TEST(Led, TurnOn_DriverFault_Propagates)
{
    led = led_create("L", PIN_A);
    mock_c()->expectOneCall("IO_write")
             ->withUnsignedIntParameters("pin_id", PIN_A)
             ->withUnsignedIntParameters("value",  IO_PIN_HIGH)
             ->andReturnIntValue(IO_ERR_HW_FAULT);

    LONGS_EQUAL(IO_ERR_HW_FAULT, led_turn_on(led));
}

/* ================================================================== */
/*  TEST GROUP: LedRgb                                                 */
/* ================================================================== */

TEST_GROUP(LedRgb)
{
    ledRgbPtr_t led;

    void setup()
    {
        pool_Init();
        led = NULL;
    }

    void teardown()
    {
        mock_c()->checkExpectations();
        mock_c()->clear();

        if(led)
        {
            led_rgb_destroy(led);
            led = NULL;
        }
    }

    ledRgbPtr_t make_rgb(const char *name = "RGB")
    {
        return led_rgb_create(name, PIN_R, PIN_G, PIN_B);
    }

    void expect_color(uint8_t r, uint8_t g, uint8_t b)
    {
        expect_write(PIN_R, r);
        expect_write(PIN_G, g);
        expect_write(PIN_B, b);
    }
};

/* ---- creation / lookup ------------------------------------------- */

TEST(LedRgb, CreateReturnsNonNull)
{
    led = make_rgb();
    CHECK(led != NULL);
}

TEST(LedRgb, GetByUuidFindsLed)
{
    led = led_rgb_createWithUuid("RGB", PIN_R, PIN_G, PIN_B, 7);
    CHECK(led != NULL);
    POINTERS_EQUAL(led, led_rgb_getByUuid(7));
}

TEST(LedRgb, DuplicateUuidReturnsNull)
{
    led = led_rgb_createWithUuid("RGB_A", PIN_R, PIN_G, PIN_B, 5);
    CHECK(led != NULL);

    ledRgbPtr_t dup = led_rgb_createWithUuid("RGB_B", PIN_R, PIN_G, PIN_B, 5);
    CHECK(dup == NULL);
}

/* ---- colour mapping --------------------------------------------- */

TEST(LedRgb, SetRed)
{
    led = make_rgb();
    expect_color(IO_PIN_HIGH, IO_PIN_LOW, IO_PIN_LOW);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_RED));
}

TEST(LedRgb, SetGreen)
{
    led = make_rgb();
    expect_color(IO_PIN_LOW, IO_PIN_HIGH, IO_PIN_LOW);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_GREEN));
}

TEST(LedRgb, SetBlue)
{
    led = make_rgb();
    expect_color(IO_PIN_LOW, IO_PIN_LOW, IO_PIN_HIGH);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_BLUE));
}

TEST(LedRgb, SetWhite)
{
    led = make_rgb();
    expect_color(IO_PIN_HIGH, IO_PIN_HIGH, IO_PIN_HIGH);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_WHITE));
}

TEST(LedRgb, SetYellow)
{
    led = make_rgb();
    expect_color(IO_PIN_HIGH, IO_PIN_HIGH, IO_PIN_LOW);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_YELLOW));
}

TEST(LedRgb, SetCyan)
{
    led = make_rgb();
    expect_color(IO_PIN_LOW, IO_PIN_HIGH, IO_PIN_HIGH);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_CYAN));
}

TEST(LedRgb, SetMagenta)
{
    led = make_rgb();
    expect_color(IO_PIN_HIGH, IO_PIN_LOW, IO_PIN_HIGH);
    LONGS_EQUAL(IO_OK, led_rgb_set(led, LED_RGB_COLOR_MAGENTA));
}

TEST(LedRgb, SetOff_AllLow)
{
    led = make_rgb();
    expect_color(IO_PIN_HIGH, IO_PIN_HIGH, IO_PIN_HIGH);   /* WHITE */
    expect_color(IO_PIN_LOW,  IO_PIN_LOW,  IO_PIN_LOW);    /* OFF   */

    led_rgb_set(led, LED_RGB_COLOR_WHITE);
    LONGS_EQUAL(IO_OK, led_rgb_off(led));
}

/* ---- state / getColor ------------------------------------------- */

TEST(LedRgb, GetColor_ReturnsLastSet)
{
    led = make_rgb();

    expect_color(IO_PIN_LOW, IO_PIN_LOW, IO_PIN_HIGH);
    led_rgb_set(led, LED_RGB_COLOR_BLUE);
    LONGS_EQUAL(LED_RGB_COLOR_BLUE, led_rgb_getColor(led));

    expect_color(IO_PIN_HIGH, IO_PIN_LOW, IO_PIN_LOW);
    led_rgb_set(led, LED_RGB_COLOR_RED);
    LONGS_EQUAL(LED_RGB_COLOR_RED, led_rgb_getColor(led));
}

TEST(LedRgb, GetColor_NullReturnsOff)
{
    LONGS_EQUAL(LED_RGB_COLOR_OFF, led_rgb_getColor(NULL));
}

/* ---- toggle ------------------------------------------------------ */
