#include "bsp/output.h"
#include "core/uprint.h"
#include "shared/pool.h"

struct output_t
{
    const char      *name;
    PWM_Interface_t *pwm;
    uint32_t         uuid;
    uint8_t          duty;
    outputPtr_t      next;
};

static outputPtr_t output_header = NULL;
static uint32_t    uuid_count    = 1U;

static void outputList_insert(outputPtr_t out);
static void outputList_delete(outputPtr_t out);
static bool outputList_uuidExists(uint32_t uuid);

/************************************************************
*                    CREATE/DESTROY                         *
*************************************************************/

outputPtr_t output_create(const char *name, PWM_Interface_t *pwm)
{
    if (pwm == NULL) return NULL;

    outputPtr_t out = (outputPtr_t)pool_Allocate();

    if (out)
    {
        out->name = name;
        out->pwm  = pwm;
        out->duty = 0U;
        out->next = NULL;

        while (outputList_uuidExists(uuid_count))
        {
            uuid_count++;
        }
        out->uuid = uuid_count++;

        out->pwm->init();
        outputList_insert(out);

        uprint("*** %s created ***\r\n", out->name);
    }
    else
    {
        uprint("Low memory, cannot create output\r\n");
    }

    return out;
}

outputPtr_t output_createWithUuid(const char *name, PWM_Interface_t *pwm, uint32_t uuid)
{
    if (pwm == NULL) return NULL;

    if (outputList_uuidExists(uuid))
    {
        uprint("Failed to create %s: UUID %u already exists\r\n", name, uuid);
        return NULL;
    }

    outputPtr_t out = (outputPtr_t)pool_Allocate();

    if (out)
    {
        out->name = name;
        out->pwm  = pwm;
        out->uuid = uuid;
        out->duty = 0U;
        out->next = NULL;

        out->pwm->init();
        outputList_insert(out);

        uprint("*** %s created with UUID %u ***\r\n", out->name, out->uuid);
    }
    else
    {
        uprint("Low memory, cannot create output\r\n");
    }

    return out;
}

outputPtr_t output_getByUuid(uint32_t uuid)
{
    outputPtr_t current = output_header;

    while (current != NULL)
    {
        if (current->uuid == uuid) return current;
        current = current->next;
    }

    return NULL;
}

void output_destroy(outputPtr_t out)
{
    if (out == NULL) return;

    output_off(out);
    if (out->pwm->deinit) out->pwm->deinit();

    uprint("*** %s destroyed ***\r\n", out->name);
    outputList_delete(out);
    pool_Free(out);
}

/************************************************************
*                        CONTROL                            *
*************************************************************/

void output_set(outputPtr_t out, uint8_t dutyPercent)
{
    if (out == NULL) return;

    if (dutyPercent > 100U) dutyPercent = 100U;

    out->duty = dutyPercent;
    out->pwm->set_duty((float)dutyPercent);
}

void output_on(outputPtr_t out)
{
    output_set(out, 100U);
}

void output_off(outputPtr_t out)
{
    output_set(out, 0U);
}

uint8_t output_getDuty(outputPtr_t out)
{
    if (out == NULL) return 0U;
    return out->duty;
}

/************************************************************
*                        DISPLAY                            *
*************************************************************/

void output_displayInfo(outputPtr_t out)
{
    if (out == NULL) return;
    uprint("Output [%s] UUID=%u duty=%u%%\r\n", out->name, out->uuid, out->duty);
}

void output_displayAll(void)
{
    outputPtr_t current = output_header;
    uprint("************************************************************\r\n");
    while (current != NULL)
    {
        uprint("  [%s] UUID=%u duty=%u%%\r\n", current->name, current->uuid, current->duty);
        current = current->next;
    }
    uprint("************************************************************\r\n");
}

/************************************************************
*                          LIST                             *
*************************************************************/

static void outputList_insert(outputPtr_t out)
{
    if (output_header == NULL)
    {
        output_header = out;
        return;
    }

    outputPtr_t current = output_header;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = out;
}

static void outputList_delete(outputPtr_t out)
{
    outputPtr_t current  = output_header;
    outputPtr_t previous = NULL;

    while (current != NULL)
    {
        if (current == out)
        {
            if (previous == NULL)
                output_header = current->next;
            else
                previous->next = current->next;
            return;
        }
        previous = current;
        current  = current->next;
    }
}

static bool outputList_uuidExists(uint32_t uuid)
{
    outputPtr_t current = output_header;
    while (current != NULL)
    {
        if (current->uuid == uuid) return true;
        current = current->next;
    }
    return false;
}
