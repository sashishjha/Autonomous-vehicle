#include "bsp.h"

volatile char framebuffer[5][5];

void scroll_left(void)
{
    for (int i = 0; i < 5; i++) {
        char first = framebuffer[i][0];
        for (int j = 0; j < 4; j++) {
            framebuffer[i][j] = framebuffer[i][j + 1];
        }
        framebuffer[i][4] = first;
    }
}

void scroll_right(void)
{
    for (int i = 0; i < 5; i++) {
        char last = framebuffer[i][4];
        for (int j = 4; j > 0; j--) {
            framebuffer[i][j] = framebuffer[i][j - 1];
        }
        framebuffer[i][0] = last;
    }
}


const int row_pins[5] = {
    LED_ROW0, LED_ROW1, LED_ROW2, LED_ROW3, LED_ROW4
};

const int col_pins[5] = {
    LED_COL0, LED_COL1, LED_COL2, LED_COL3, LED_COL4
};

void display(const char pic[5][5])
{
    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 5; j++)
            framebuffer[i][j] = pic[i][j];
}


void display_row_refresh(void)
{
    static uint8_t row = 0;

    for (int i = 0; i < 5; i++)
        gpio_write(row_pins[i], 0);

    for (int j = 0; j < 5; j++)
        gpio_write(col_pins[j], 1);

    gpio_write(row_pins[row], 1);

    for (int j = 0; j < 5; j++)
    {
        if (framebuffer[row][j] == '1')
            gpio_write(col_pins[j], 0);
    }

    row++;
    if (row == 5)
        row = 0;
}


