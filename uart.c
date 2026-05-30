#include <stdint.h>
#include "nrf52833.h"

void uart_init(uint32_t rxpin, uint32_t txpin)
{
    NRF_UART0->PSEL.RXD = rxpin;
    NRF_UART0->PSEL.TXD = txpin;
    NRF_UART0->PSEL.RTS = 0xFFFFFFFF;
    NRF_UART0->PSEL.CTS = 0xFFFFFFFF;

    NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
    NRF_UART0->CONFIG   = 0;

    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
}

void uart_putc(char ch)
{
    NRF_UART0->TXD = (uint8_t)ch;
    while (NRF_UART0->EVENTS_TXDRDY == 0);
    NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_puts(char s[])
{
    while (*s)
    {
        uart_putc(*s++);
    }
}


/* Check if a new byte has been received */
int uart_read_ready(void)
{
    return NRF_UART0->EVENTS_RXDRDY;
}

/* Read received byte */
char uart_read_char(void)
{
    char ch = (char)NRF_UART0->RXD;
    NRF_UART0->EVENTS_RXDRDY = 0;  // Clear event
    return ch;
}
