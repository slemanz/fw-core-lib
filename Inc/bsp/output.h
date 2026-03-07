#ifndef INC_OUTPUT_H_
#define INC_OUTPUT_H_

#include <stdint.h>
#include <stdbool.h>
#include "interface/interface.h"

typedef struct output_t *outputPtr_t;

/************************************************************
*                    CREATE/DESTROY                         *
*************************************************************/

outputPtr_t output_create(const char *name, PWM_Interface_t *pwm);
outputPtr_t output_createWithUuid(const char *name, PWM_Interface_t *pwm, uint32_t uuid);
outputPtr_t output_getByUuid(uint32_t uuid);
void        output_destroy(outputPtr_t out);

/************************************************************
*                        CONTROL                            *
*************************************************************/

void    output_set(outputPtr_t out, uint8_t dutyPercent);
void    output_on(outputPtr_t out);
void    output_off(outputPtr_t out);
uint8_t output_getDuty(outputPtr_t out);

/************************************************************
*                        DISPLAY                            *
*************************************************************/

void output_displayInfo(outputPtr_t out);
void output_displayAll(void);

#endif /* INC_OUTPUT_H_ */