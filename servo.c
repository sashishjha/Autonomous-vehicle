#include <stdint.h>
#include "nrf52833.h"
#include "servo.h"

#define PWM         NRF_PWM2
#define PWM_CLK     1000000     // 1 MHz
#define SERVO_FREQ  50          // 50 Hz

#define calib_offset 15

static uint16_t seq[1];
static int countertop;

void servo_init(int pin)
{
    /* prescaler 16 ? 1 MHz clock */
    PWM->PRESCALER = 4;

    /* 50 Hz period */
    countertop = PWM_CLK / SERVO_FREQ;
    PWM->COUNTERTOP = countertop;

    /* connect servo pin */
    PWM->PSEL.OUT[0] = pin;

    PWM->DECODER = 0;

    PWM->ENABLE = 1;
}

void servo_set_angle(int angle)
{
		int adj_angle = angle + calib_offset; 
    if (adj_angle < 0) adj_angle = 0;
    if (adj_angle > 180) adj_angle = 180;

    /* convert angle ? pulse width */
    int pulse = 500 + (adj_angle * 2000) / 180;   // 1–2 ms

    uint16_t counter = (countertop * pulse) / 20000;

#define POLARITY (1 << 15)

    seq[0] = POLARITY | counter;

    PWM->SEQ[0].PTR = (uint32_t)seq;
    PWM->SEQ[0].CNT = 1;
    PWM->SEQ[0].REFRESH = 0;

    PWM->EVENTS_SEQSTARTED[0] = 0;
    PWM->TASKS_SEQSTART[0] = 1;

    while (!PWM->EVENTS_SEQSTARTED[0]);
}




