#include <stdarg.h>
#include <stdint.h>
#include "printf.h"

/* provided elsewhere */
extern void uart_putc(char ch);

static void out_char(char c)
{
    uart_putc(c);
}

static void out_str(const char *s)
{
    while (*s)
        out_char(*s++);
}

static void print_uint(uint32_t val, uint32_t base)
{
    char buf[16];
    int i = 0;

    if (val == 0)
    {
        out_char('0');
        return;
    }

    while (val && i < (int)sizeof(buf))
    {
        uint32_t d = val % base;
        buf[i++] = (d < 10) ? ('0' + d) : ('a' + d - 10);
        val /= base;
    }

    while (i--)
        out_char(buf[i]);
}

static void print_int(int32_t val)
{
    if (val < 0)
    {
        out_char('-');
        val = -val;
    }
    print_uint((uint32_t)val, 10);
}

static void print_float(float d)
{
    if (d < 0.0)
    {
        out_char('-');
        d = -d;
    }

    int32_t ip = (int32_t)d;
    float frac = d - (float)ip;

    print_int(ip);
    out_char('.');

    int32_t digit = (int32_t)(frac * 10.0 + 0.5);
    out_char('0' + digit);
}

int puts(const char *s)
{
    out_str(s);
    out_char('\n');
    return 1;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    int count = 0;

    va_start(ap, fmt);

    while (*fmt)
    {
        if (*fmt != '%')
        {
            out_char(*fmt++);
            count++;
            continue;
        }

        fmt++;

        switch (*fmt)
        {
        case 'd':
            print_int(va_arg(ap, int));
            break;

        case 'x':
            print_uint(va_arg(ap, unsigned int), 16);
            break;

        case 'c':
            out_char((char)va_arg(ap, int));
            break;

        case 's':
            out_str(va_arg(ap, char *));
            break;

        case 'f':
            print_float(va_arg(ap, double));
            break;

        case '%':
            out_char('%');
            break;

        default:
            out_char('?');
            break;
        }

        fmt++;
    }

    va_end(ap);
    return count;
}
