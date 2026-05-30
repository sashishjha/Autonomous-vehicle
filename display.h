/* display.h */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void display(const char pic[5][5]);
void display_row_refresh(void);
void scroll_left(void);
void scroll_right(void);

#endif