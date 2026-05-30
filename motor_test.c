#include "microbit.h"
#include "motor.h"
#include <stdint.h>

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int main(void)
{
    motor_init(M1A, M1B, M2A, M2B);

    while(1)
    {
        motor_on(MOTOR_FORWARD, 40, MOTOR_FORWARD, 40);
        delay_ms(1000);
        motor_off();
        delay_ms(1000);

        motor_on(MOTOR_REVERSE, 40, MOTOR_REVERSE, 40);
        delay_ms(1000);
        motor_off();
        delay_ms(2000);
    }
}

void encoder1_callback(void) {}
void encoder2_callback(void) {}
