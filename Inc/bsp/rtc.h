#ifndef INC_RTC_H_
#define INC_RTC_H_

#include <stdint.h>
#include "interface/interface.h"

#define DS3231_I2C_ADDRESS      0x68

/* Register address */
#define DS3231_ADDR_SEC         0x00
#define DS3231_ADDR_MIN         0x01
#define DS3231_ADDR_HRS         0x02
#define DS3231_ADDR_DAY         0x03
#define DS3231_ADDR_DATE        0x04
#define DS3231_ADDR_MONTH       0x05
#define DS3231_ADDR_YEAR        0x06

#define TIME_FORMAT_12HRS_AM	0
#define TIME_FORMAT_12HRS_PM	1
#define TIME_FORMAT_24HRS		2

// sunday can be defined as any day, only keep logic sequential!
typedef enum
{
	SUNDAY = 1,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
} RTC_DayOfWeek_e;

typedef struct
{
	uint8_t date;
	uint8_t month;
	uint8_t year;
	RTC_DayOfWeek_e day;
}RTC_date_t;

typedef struct
{
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t time_format;
}RTC_time_t;

typedef struct
{
	RTC_date_t date;
	RTC_time_t time;
}RTC_DateTime_t;

uint8_t binary_to_bcd(uint8_t value);
uint8_t bcd_to_binary(uint8_t value);

void rtc_setup(uint8_t comm_id);

void rtc_set_current_time(RTC_time_t *rtc_time);
void rtc_get_current_time(RTC_time_t *rtc_time);

void rtc_set_current_date(RTC_date_t *rtc_date);
void rtc_get_current_date(RTC_date_t *rtc_date);

void rtc_set(RTC_DateTime_t *rtc_dateTime);
void rtc_get(RTC_DateTime_t *rtc_dateTime);

#endif /* INC_RTC_H_ */