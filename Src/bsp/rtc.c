#include "bsp/rtc.h"

uint8_t rtc_comm_id = 0;

static void rtc_write(uint8_t value, uint8_t reg_addr);
static uint8_t rtc_read(uint8_t reg_addr);


uint8_t binary_to_bcd(uint8_t value)
{
    uint8_t m, n;
    uint8_t bcd;

    bcd = value;
    if(value >= 10)
    {
        m = value/10;
        n = value % 10;
        bcd = (uint8_t)((m << 4) | n);
    }

    return bcd;
}

uint8_t bcd_to_binary(uint8_t value)
{
    uint8_t m, n;
    uint8_t bin;

    m = (uint8_t)((value >> 4) * 10);
    n = value & (uint8_t)0x0F;

    bin = m + n;
    return bin;
}



void rtc_setup(uint8_t comm_id)
{
    rtc_comm_id = comm_id;
}

void rtc_set_current_time(RTC_time_t *rtc_time)
{
    uint8_t seconds, hrs;
	seconds = binary_to_bcd(rtc_time->seconds);
	seconds &= ~(1 << 7);

	rtc_write(seconds, DS3231_ADDR_SEC);
	rtc_write(binary_to_bcd(rtc_time->minutes), DS3231_ADDR_MIN);

	hrs = binary_to_bcd(rtc_time->hours);

	if(rtc_time->time_format == TIME_FORMAT_24HRS)
	{
		hrs &= ~(1 << 6);
	}else
	{
		hrs |= (1 << 6);
		hrs = (rtc_time->time_format == TIME_FORMAT_12HRS_PM) ? hrs | (1 << 5) : hrs & ~(1 << 5);
	}

	rtc_write(hrs, DS3231_ADDR_HRS);
}

void rtc_get_current_time(RTC_time_t *rtc_time)
{
    uint8_t seconds, hrs;

	seconds = rtc_read(DS3231_ADDR_SEC);

	seconds &= ~(1 << 7);
	rtc_time->seconds = bcd_to_binary(seconds);
	rtc_time->minutes = bcd_to_binary(rtc_read(DS3231_ADDR_MIN));

    hrs = rtc_read(DS3231_ADDR_HRS);
	if(hrs & (1 << 6))
	{
		// 12 hr format
		rtc_time->time_format = !((hrs & (1 << 5)) == 0);
		hrs &= ~(0x3 << 5); /// clear 6 and 5
	}else
	{
		// 24 hr format
		rtc_time->time_format = TIME_FORMAT_24HRS;
	}
	rtc_time->hours = bcd_to_binary(hrs);
}

void rtc_set_current_date(RTC_date_t *rtc_date)
{
    rtc_write(binary_to_bcd(rtc_date->date), DS3231_ADDR_DATE);
    rtc_write(binary_to_bcd(rtc_date->month), DS3231_ADDR_MONTH);
    rtc_write(binary_to_bcd(rtc_date->year), DS3231_ADDR_YEAR);
    rtc_write(binary_to_bcd(rtc_date->day), DS3231_ADDR_DAY);
}

void rtc_get_current_date(RTC_date_t *rtc_date)
{
    rtc_date->day   = bcd_to_binary(rtc_read(DS3231_ADDR_DAY));
    rtc_date->date  = bcd_to_binary(rtc_read(DS3231_ADDR_DATE));
    rtc_date->month = bcd_to_binary(rtc_read(DS3231_ADDR_MONTH));
    rtc_date->year  = bcd_to_binary(rtc_read(DS3231_ADDR_YEAR));
}

void rtc_set(RTC_DateTime_t *rtc_dateTime)
{
    rtc_set_current_time(&(rtc_dateTime->time));
    rtc_set_current_date(&(rtc_dateTime->date));
}

void rtc_get(RTC_DateTime_t *rtc_dateTime)
{
    rtc_get_current_time(&(rtc_dateTime->time));
    rtc_get_current_date(&(rtc_dateTime->date));
}

static void rtc_write(uint8_t value, uint8_t reg_addr)
{
    uint8_t write_buffer[3] = {DS3231_I2C_ADDRESS, reg_addr, value};

    comm_send(rtc_comm_id, write_buffer, 3);
}

static uint8_t rtc_read(uint8_t reg_addr)
{
    uint8_t write_buffer[2] = {DS3231_I2C_ADDRESS, reg_addr};
    uint8_t read_buffer;

    comm_send(rtc_comm_id, write_buffer, 2);
    comm_send(rtc_comm_id, write_buffer, 1);
    comm_receive(rtc_comm_id, &read_buffer, 1);

    return read_buffer;

}