/* ledbtn.c */

#include "microbit.h"
#include "ledbtn.h"
#include "gpio.h"

/* row and column pin numbers */
static const int row_pins[5] = {
    LED_ROW0, LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4
};

static const int col_pins[5] = {
    LED_COL0, LED_COL1, LED_COL2, LED_COL3, LED_COL4
};

/* button pin numbers */
static const int button_pins[] = {
    BUTTON_0,
    BUTTON_1
};

void led_init(void)
{
    for (int r = 0; r < 5; r++) {
        gpio_config(row_pins[r], GPIO_OUTPUT);
        gpio_write(row_pins[r], 0);
    }

    for (int c = 0; c < 5; c++) {
        gpio_config(col_pins[c], GPIO_OUTPUT);
        gpio_write(col_pins[c], 1);
    }
}

void led_on(int row, int col)
{
    gpio_write(row_pins[row], 1);
    gpio_write(col_pins[col], 0);
}

void led_off(int row, int col)
{
    gpio_write(col_pins[col], 1);
    gpio_write(row_pins[row], 0);
}

void button_init(void)
{
    for (unsigned i = 0; i < sizeof(button_pins)/sizeof(button_pins[0]); i++) {
        gpio_config(button_pins[i], GPIO_INPUT);
    }
}

int button_read(int id)
{
    return gpio_read(button_pins[id]) == 0;
}
