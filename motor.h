#ifndef MOTOR_H
#define MOTOR_H

void motor_init(int m1a, int m1b, int m2a, int m2b);
void motor_on(int dirA, int dutyA, int dirB, int dutyB);
void motor_off(void);
enum { MOTOR_FORWARD, MOTOR_REVERSE };

#endif /* MOTOR_H */
