#ifndef INC_INTERFACE_H_
#define INC_INTERFACE_H_

#include <stdint.h>
#include <stddef.h>

/************************************************************
*                       TIMEBASE                            *
*************************************************************/

uint64_t timebase_get   (void);
void     timebase_deinit(void);

/************************************************************
*                       COMM                                *
*************************************************************/

void    comm_init          (uint8_t comm_id);
void    comm_send          (uint8_t comm_id, uint8_t *buf, uint32_t len);
uint8_t comm_receive       (uint8_t comm_id, uint8_t *buf, uint32_t len);
uint8_t comm_data_available(uint8_t comm_id);
void    comm_deinit        (uint8_t comm_id);

/************************************************************
*                       IO                                  *
*************************************************************/

typedef enum
{
    IO_OK              =  0,
    IO_ERR_NULL        = -1,
    IO_ERR_INVALID_PIN = -2,
    IO_ERR_NOT_INIT    = -3,
    IO_ERR_HW_FAULT    = -4,
    IO_ERR_TIMEOUT     = -5,
}io_status_t;

typedef enum
{
    IO_OPT_MODE        = 0u,
    IO_OPT_PULL        = 1u,
    IO_OPT_SPEED       = 2u,
    IO_OPT_OUTPUT_TYPE = 3u,
} io_option_e;

/* Values for IO_OPT_PULL */
#define IO_PULL_NONE   ((uint8_t)0u)   
#define IO_PULL_UP     ((uint8_t)1u)   
#define IO_PULL_DOWN   ((uint8_t)2u)   

/* Values for IO_OPT_SPEED */
#define IO_SPEED_LOW    ((uint8_t)0u)  
#define IO_SPEED_MEDIUM ((uint8_t)1u)  
#define IO_SPEED_FAST   ((uint8_t)2u)  
#define IO_SPEED_HIGH   ((uint8_t)3u)  

/* Values for IO_OPT_OUTPUT_TYPE */
#define IO_OTYPE_PUSH_PULL  ((uint8_t)0u)
#define IO_OTYPE_OPEN_DRAIN ((uint8_t)1u)

#define IO_PIN_HIGH     ((uint8_t)1u)
#define IO_PIN_LOW      ((uint8_t)0u)

#define IO_MODE_INPUT   ((uint8_t)0u)
#define IO_MODE_OUTPUT  ((uint8_t)1u)

io_status_t IO_init(uint8_t pin_id);
io_status_t IO_write(uint8_t pin_id, uint8_t value);
io_status_t IO_read(uint8_t pin_id, uint8_t *out_value);
io_status_t IO_toggle(uint8_t pin_id);
io_status_t IO_configure(uint8_t pin_id, uint8_t option, uint8_t value);
io_status_t IO_deinit(uint8_t pin_id);

/************************************************************
*                       ADC                                 *
*************************************************************/

void     analog_init  (uint8_t channel_id);
uint16_t analog_read  (uint8_t channel_id);
void     analog_deinit(uint8_t channel_id);

/************************************************************
*                       PWM                                 *
*************************************************************/

void PWM_init    (uint8_t instance_id);
void PWM_set_duty(uint8_t instance_id, float duty_percent);
void PWM_deinit  (uint8_t instance_id);

#endif /* INC_INTERFACE_H_ */