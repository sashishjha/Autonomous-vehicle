#include "microbit.h"
#include "printf.h"
#include "uart.h"

#ifndef UART_RX_PIN
#define UART_RX_PIN UART_RX
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN UART_TX
#endif

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int main(void)
{
    uart_init(UART_RX_PIN, UART_TX_PIN);
    printf("\r\nSERIAL TEST BOOT\r\n");

    while(1)
    {
        if(uart_read_ready())
        {
            char c = uart_read_char();
            printf("GOT: %c\r\n", c);

            if(c == '\n')
                printf("DONE\r\n");
        }

        delay_ms(1);
    }
}

void encoder1_callback(void) {}
void encoder2_callback(void) {}
