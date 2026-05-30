/* ledbtn.h */

#ifndef LEDBTN_H
#define LEDBTN_H

void led_init(void);
void led_on(int row, int col);
void led_off(int row, int col);

void button_init(void);
int  button_read(int id);

#endif
