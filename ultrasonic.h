#ifndef ULTRASONIC_H
#define ULTRASONIC_H

void ultrasonic_init(int trig_pin, int echo_pin);
int ultrasonic_read_cm(void);

#endif