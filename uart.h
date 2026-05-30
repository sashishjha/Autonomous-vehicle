#ifndef UART_H
#define UART_H

#include <stdint.h>

extern void uart_init(uint32_t rxpin, uint32_t txpin);
extern void uart_putc(char ch);
extern void uart_puts(char s[]);

/* --- NEW: RX SUPPORT --- */
extern int  uart_read_ready(void);
extern char uart_read_char(void);

#endif /* UART_H */
