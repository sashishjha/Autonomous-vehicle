#include "gpio.h"
#include "ultrasonic.h"

static int trig;
static int echo;

static void delay(volatile int d)
{
    while(d--);
}

void ultrasonic_init(int trig_pin, int echo_pin)
{
    trig = trig_pin;
    echo = echo_pin;

    gpio_config(trig, GPIO_OUTPUT);
    gpio_config(echo, GPIO_INPUT);

    gpio_write(trig,0);
}

int ultrasonic_read_cm(void)
{
    int count = 0;

    /* trigger pulse */
    gpio_write(trig,1);
    delay(200);
    gpio_write(trig,0);

    /* wait for echo start */
    int timeout = 200000;
    while(!gpio_read(echo) && timeout--)
        ;

    /* measure echo pulse */
    while(gpio_read(echo))
    {
        count++;
        if(count > 50000)
            break;
    }

    /* rough conversion */
    int distance_cm = count / 58;

    return distance_cm;
}
