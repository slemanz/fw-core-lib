#include "core/uprint.h"

static uint8_t s_comm_id = 0;

static void uprint_send(char c)
{
    comm_send(s_comm_id, (uint8_t*)&c, 1);
}

static void uprint_puts(const char *s)
{
    while(*s) uprint_send(*s++);
}

static void uprint_putu(uint32_t n, uint8_t base)
{
    char buf[12];
    char *p = buf + sizeof(buf);

    const char *digits = "0123456789ABCDEF";

    *(--p) = '\0';
    if(n == 0)
    {
        *(--p) = '0';
    }else
    {
        while(n){
            *(--p) = digits[n % base];
            n /= base;
        }
    }

    uprint_puts(p);
}

static void uprint_puti(int32_t n)
{
    if(n < 0)
    {
        uprint_send('-');
        /* Cast to unsigned before negating to handle INT32_MIN safely */
        uprint_putu((uint32_t)(-(int64_t)n), 10);
        return;
    }
    uprint_putu((uint32_t)n, 10);
}

void uprint_setup(uint8_t comm_id)
{
    s_comm_id = comm_id;
}

void uprint(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    while(*fmt)
    {
        if(*fmt == '%')
        {
            switch (*(++fmt))
            {
                case 'd': uprint_puti(va_arg(args, int)); break;
                case 'u': uprint_putu(va_arg(args, unsigned), 10); break;
                case 'x': uprint_putu(va_arg(args, unsigned), 16); break;
                case 's': uprint_puts(va_arg(args, char*)); break;
                case 'c': uprint_send(va_arg(args, int)); break;
                case '%': uprint_send('%'); break;
                default:  uprint_send(*fmt); break;
            }
            fmt++;
        }else
        {
            uprint_send(*fmt++);
        }
    }

    va_end(args);
}