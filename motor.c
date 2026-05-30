#include <stdint.h>
#include "nrf52833.h"
#include "motor.h"

static enum { MOTOR_OFF, MOTOR_ON } motor_status = MOTOR_OFF;
static int countertop;

#define PWM         NRF_PWM3
#define PWM_CLK     1000000
#define PWM_FREQ    500

void motor_init(int m1A, int m1B, int m2A, int m2B)
{
    /* Set prescaler divisor = 16 for 1 MHz clock */
    PWM->PRESCALER = 4;     // 16 = 1 << 4

    /* Set period */
    countertop = PWM_CLK / PWM_FREQ;
    PWM->COUNTERTOP = countertop;

    /* Connect output pins */
    PWM->PSEL.OUT[0] = m1A;
    PWM->PSEL.OUT[1] = m1B;
    PWM->PSEL.OUT[2] = m2A;
    PWM->PSEL.OUT[3] = m2B;

    /* Individual duty cycles for each channel */
    PWM->DECODER = 2;

    PWM->ENABLE = 1;
}

void motor_on(int dir1, int duty1, int dir2, int duty2)
{
    static uint16_t s_sequence[4];  // this cannot be on the stack
                                    // because DMA can access it after
                                    // the function returns
		uint16_t counter1, counter2;

		// set counter values proportional to duty cycle.
		counter1 = (countertop * duty1) / 100;
		counter2 = (countertop * duty2) / 100;

    switch (dir1)
    {
#define POLARITY (1 << 15)	// high to low (counter value corresponds to ON time)

        case MOTOR_FORWARD:
            s_sequence[0] = POLARITY | counter1;
            s_sequence[1] = POLARITY | 0;
            break;

        case MOTOR_REVERSE:
            s_sequence[0] = POLARITY | 0;
            s_sequence[1] = POLARITY | counter1;
            break;

        default:
            break;
    }

    switch (dir2)
    {
        case MOTOR_FORWARD:
            s_sequence[2] = POLARITY | counter2;
            s_sequence[3] = POLARITY | 0;
            break;

        case MOTOR_REVERSE:
            s_sequence[2] = POLARITY | 0;
            s_sequence[3] = POLARITY | counter2;
            break;

        default:
            break;
    }

    PWM->SEQ[0].PTR = (uint32_t) s_sequence;
    PWM->SEQ[0].CNT = 4;            // one value per channel
    PWM->SEQ[0].REFRESH = 0;        // continuous

		PWM->EVENTS_SEQSTARTED[0] = 0;
    PWM->TASKS_SEQSTART[0] = 1;
    while (PWM->EVENTS_SEQSTARTED[0] == 0)
        ;

    motor_status = MOTOR_ON;

    return;
}

void motor_off(void)
{
    if (motor_status == MOTOR_OFF)
        return;

    PWM->TASKS_STOP = 1;
    while (PWM->EVENTS_STOPPED == 0)
        ;

    PWM->EVENTS_STOPPED = 0;

    motor_status = MOTOR_OFF;

    return;
}
