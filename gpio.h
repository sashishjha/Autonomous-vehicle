#ifndef GPIO_H
#define GPIO_H

/* KEEP YOUR EXISTING ENUM */
typedef enum {
    GPIO_INPUT,
    GPIO_OUTPUT
} gpio_dir_t;

/* YOUR EXISTING FUNCTIONS */
void gpio_config(int pin, gpio_dir_t dir);
void gpio_write(int pin, int value);
int  gpio_read(int pin);

/* THE NEW INTERRUPT FUNCTION */
void gpio_inten(int pin, int event_no, int edge);

#endif