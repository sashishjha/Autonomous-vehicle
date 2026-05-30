/* gpio.c */

#include "gpio.h"
#include "nrf52833.h"

static inline NRF_GPIO_Type *gpio_port(int pin)
{
    if (pin & 0x20) {
        return NRF_P1;
    } else {
        return NRF_P0;
    }
}

static inline int gpio_bit(int pin)
{
    return pin & 0x1F;
}

void gpio_init(void)
{
}

void gpio_config(int pin, gpio_dir_t dir)
{
    NRF_GPIO_Type *port = gpio_port(pin);
    int bit = gpio_bit(pin);

    if (dir == GPIO_OUTPUT) {
        port->PIN_CNF[bit] =
            (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
            (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
            (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos);
    } else {
        port->PIN_CNF[bit] =
            (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
            (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
            (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos);
    }
}

void gpio_write(int pin, int value)
{
    NRF_GPIO_Type *port = gpio_port(pin);
    int bit = gpio_bit(pin);

    if (value) {
        port->OUTSET = (1UL << bit);
    } else {
        port->OUTCLR = (1UL << bit);
    }
}

int gpio_read(int pin)
{
    NRF_GPIO_Type *port = gpio_port(pin);
    int bit = gpio_bit(pin);

    return (port->IN >> bit) & 1U;
}

/* ----------------------------------------------------------- */
/* NEW: Encoder Interrupt Support (GPIOTE)                     */
/* ----------------------------------------------------------- */

extern void encoder1_callback(void);
extern void encoder2_callback(void);

void gpio_inten(int pin, int event_no, int edge)
{
    // Configure GPIOTE Channel using your existing nrf52833.h macros
    NRF_GPIOTE->CONFIG[event_no] = (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
                                   (pin << GPIOTE_CONFIG_PSEL_Pos) |
                                   (edge << GPIOTE_CONFIG_POLARITY_Pos);

    NRF_GPIOTE->INTENSET = (1UL << event_no);

    // Hardcoded NVIC Enable (Address: 0xE000E100, IRQ ID: 6)
    *(volatile uint32_t *)0xE000E100 = (1UL << 6);
    
    // Hardcoded NVIC Priority (Address: 0xE000E404, Priority 3 for RTOS)
    // We clear bits 16-23 and set them to 0x60 (Priority 3)
    uint32_t priority_reg = *(volatile uint32_t *)0xE000E404;
    priority_reg &= ~(0xFFUL << 16);
    priority_reg |= (0x60UL << 16);
    *(volatile uint32_t *)0xE000E404 = priority_reg;
}

void GPIOTE_IRQHandler(void)
{
    for (int i = 0; i < 8; i++)
    {
        if (NRF_GPIOTE->EVENTS_IN[i])
        {
            NRF_GPIOTE->EVENTS_IN[i] = 0; // Clear the event
            if (i == 0) encoder1_callback();
            if (i == 1) encoder2_callback();
        }
    }
}