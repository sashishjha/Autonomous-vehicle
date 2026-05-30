#ifndef MICROBIT_H
#define MICROBIT_H

/* Pin definitions for microbit v2 */

/* LEDs definitions for 5x5 matrix display */
#define LED_NUM_ROWS    5
#define LED_NUM_COLS    5

#define LED_ROW0    21
#define LED_ROW1    22
#define LED_ROW2    15
#define LED_ROW3    24
#define LED_ROW4    19

#define LED_COL0    28
#define LED_COL1    11
#define LED_COL2    31
#define LED_COL3    37
#define LED_COL4    30

/* Buttons */
#define BUTTONS_NUMBER  2

#define BUTTON_A    0
#define BUTTON_B    1

#define BUTTON_0    14
#define BUTTON_1    23

#define BUTTONS_ACTIVE_STATE    0

/* UART */
#define UART_RX     40   // P1_08
#define UART_TX     6    // P0_06

/* I2C */
#define I2C_SCL     8    // P0_08
#define I2C_SDA     16   // P0_16
#define I2C_INT     25   // P0_25

/* On-board speaker and mic */
#define SPEAKER     0   // P0_00
#define MIC         3   // AIN3 on P0_05
                        //  (we need to specify analog input channel,
                        //   and not the pin number)
#define RUN_MIC     20  // P0_20

/* Edge connector pins */
#define P0 2        // ring0
#define P1 3        // ring1
#define P2 4        // ring2
#define P8 10       // GPIO1 for NFC
#define P9 9        // GPIO2 for NFC
#define P12 12      // GPIO4 for accessibility

#define P13 17      // Ext SPI clock
#define P14 01      // Ext SPI MISO
#define P15 13      // Ext SPI MOSI
#define P16 34      // GPIO3

#define P19 26      // I2C_EXT_SCL
#define P20 32      // I2C_EXT_SDA

#define P5 14
#define P7 11
#define P10 30
#define P11 23
#define P3 31
#define P4 28




#define M1A P1
#define M1B P2
#define M2A P19
#define M2B P20

#define E1A P13
#define E1B P14
#define E2A P15
#define E2B P16


#define TRIG P3
#define ECHO P4
#define SERVO P12
#define IR4 P5
#define IR5 P7
#define IR6 P0
#define IR7 P10
#define IR8 P11

#endif /* MICROBIT_H */