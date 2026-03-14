#include "bsp/output.h"
#include "core/uprint.h"
#include "shared/pool.h"
#include "shared/slist.h"

struct output_t
{
    const char      *name;
    uint8_t         pwm_id;
    uint32_t        uuid;
    uint8_t         duty;
    slist_node_t    node;
};

static slist_head_t s_output_list;
static uint32_t     s_uuid_count = 1U;

static bool output_uuid_exists(uint32_t uuid);

/************************************************************
*                    CREATE/DESTROY                         *
*************************************************************/

outputPtr_t output_create(const char *name, uint8_t pwm_id)
{
    outputPtr_t out = (outputPtr_t)pool_Allocate();

    if (out)
    {
        out->name = name;
        out->pwm_id  = pwm_id;
        out->duty = 0U;

        while (output_uuid_exists(s_uuid_count))
        {
            s_uuid_count++;
        }
        out->uuid = s_uuid_count++;

        PWM_init(pwm_id);
        slist_push_front(&s_output_list, &out->node);

        uprint("*** %s created ***\r\n", out->name);
    }
    else
    {
        uprint("Low memory, cannot create output\r\n");
    }

    return out;
}

outputPtr_t output_createWithUuid(const char *name, uint8_t pwm_id, uint32_t uuid)
{
    if (output_uuid_exists(uuid))
    {
        uprint("Failed to create %s: UUID %u already exists\r\n", name, uuid);
        return NULL;
    }

    outputPtr_t out = (outputPtr_t)pool_Allocate();

    if (out)
    {
        out->name = name;
        out->pwm_id  = pwm_id;
        out->uuid = uuid;
        out->duty = 0U;

        PWM_init(pwm_id);
        slist_push_front(&s_output_list, &out->node);

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
    slist_node_t *it;
    slist_for_each(it, &s_output_list)
    {
        outputPtr_t out = container_of(it, struct output_t, node);
        if (out->uuid == uuid) return out;
    }
    return NULL;
}

void output_destroy(outputPtr_t out)
{
    if (out == NULL) return;

    output_off(out);
    PWM_deinit(out->pwm_id);

    uprint("*** %s destroyed ***\r\n", out->name);
    slist_remove(&s_output_list, &out->node);
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
    PWM_set_duty(out->pwm_id, dutyPercent);
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
    uprint("************************************************************\r\n");
    slist_node_t *it;
    slist_for_each(it, &s_output_list)
    {
        outputPtr_t out = container_of(it, struct output_t, node);
        uprint("  [%s] UUID=%u duty=%u%%\r\n", out->name, out->uuid, out->duty);
    }
    uprint("************************************************************\r\n");
}

/************************************************************
*                       LIST HELPERS                        *
*************************************************************/

static bool output_uuid_exists(uint32_t uuid)
{
    slist_node_t *it;
    slist_for_each(it, &s_output_list)
    {
        outputPtr_t out = container_of(it, struct output_t, node);
        if (out->uuid == uuid) return true;
    }
    return false;
}
