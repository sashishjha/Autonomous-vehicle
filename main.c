#ifdef ROLE_BOT

#include "microbit.h"
#include "radio.h"
#include "motor.h"
#include "gpio.h"
#include "pins.h"
#include "ultrasonic.h"
#include <stdbool.h>

/* --- Parameters --- */
#define PPR 142
#define MOTION_TIMEOUT_TICKS 500

#define TURN_GAIN_L 2.2f
#define TURN_GAIN_R 2.2f

#define MODE_LINEAR 1
#define MODE_TURN   2
#define MODE_AUTONOMOUS 3
#define MODE_CIRCLE 4

#define WHEEL_CIRCUMFERENCE_CM 20.73f   // 6.6 * 3.1416
#define DISTANCE_GAIN 1.0f              // tune this later like TURN_GAIN

/* --- Global State --- */
volatile int enc1_count = 0;
volatile int enc2_count = 0;

typedef struct {
    char cmd;
    int value;
} command_t;

/* --- MOTION STATE --- */
typedef struct {
    int active;
    int mode;
    char cmd;
    int start_l;
    int start_r;
    int target;
    int timeout;
    int last_progress;
} motion_state_t;

motion_state_t motion = {0};

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int iabs_int(int value)
{
    return (value < 0) ? -value : value;
}

int cm_to_ticks(float cm)
{
    float rev = cm / WHEEL_CIRCUMFERENCE_CM;
    return (int)(rev * PPR);
}

/* ----------------------------------------------------------- */
/* Encoder Callbacks                                           */
/* ----------------------------------------------------------- */

void encoder1_callback(void) {
    if (gpio_read(E1B)) enc1_count++;
    else enc1_count--;
}

void encoder2_callback(void) {
    if (gpio_read(E2B)) enc2_count++;
    else enc2_count--;
}

/* ----------------------------------------------------------- */
/* SEND DONE                                                   */
/* ----------------------------------------------------------- */

void send_done(void)
{
    map_packet_t pkt;

    pkt.id = 4;
    pkt.angle = 0;
    pkt.distance = 0;
    pkt.encoder_left = enc1_count;
    pkt.encoder_right = enc2_count;
    pkt.reserved = 0;

    radio_tx_map(&pkt);
}

/* ----------------------------------------------------------- */
/* SEND ACK                                                    */
/* ----------------------------------------------------------- */

void send_ack(void)
{
    map_packet_t pkt;

    pkt.id = 3;
    pkt.angle = 0;
    pkt.distance = 0;
    pkt.encoder_left = enc1_count;
    pkt.encoder_right = enc2_count;
    pkt.reserved = 0;

    radio_tx_map(&pkt);
}

void start_motion(command_t cmd_msg)
{
    char cmd = cmd_msg.cmd;

    if(cmd == 'S')
    {
        motor_off();
        motion.active = 0;
        return;
    }

    if(motion.active)
        return;

    motion.cmd = cmd;
    motion.start_l = enc1_count;
    motion.start_r = enc2_count;
    motion.timeout = 0;
    motion.active = 1;
    motion.last_progress = 0;

    if(cmd == 'F' || cmd == 'B')
    {
        motion.mode = MODE_LINEAR;
        if(cmd_msg.value > 0)
            motion.target = (int)(cm_to_ticks(cmd_msg.value) * DISTANCE_GAIN);
        else
            motion.target = (int)(cm_to_ticks(5.0f) * DISTANCE_GAIN);

        if(cmd == 'F')
            motor_on(MOTOR_FORWARD, 40, MOTOR_FORWARD, 40);
        else
            motor_on(MOTOR_REVERSE, 40, MOTOR_REVERSE, 40);

        send_ack();
    }
    else if(cmd == 'C')
    {
        motion.mode = MODE_CIRCLE;
        motion.target = (int)(cm_to_ticks(cmd_msg.value > 0 ? cmd_msg.value : 50) * DISTANCE_GAIN);
        motor_on(MOTOR_FORWARD, 20, MOTOR_FORWARD, 50); // Left slow, right fast = circle left
        send_ack();
    }
    else if(cmd == 'A')
    {
        motion.mode = MODE_AUTONOMOUS;
        motion.target = 0; // runs indefinitely
        motor_on(MOTOR_FORWARD, 35, MOTOR_FORWARD, 35);
        send_ack();
    }
    else if(cmd == 'L' || cmd == 'R')
    {
        motion.mode = MODE_TURN;

        if(cmd == 'L')
        {
            motion.target = (int)((90.0f * PPR / 360.0f) * TURN_GAIN_L);
            motor_on(MOTOR_REVERSE, 45, MOTOR_FORWARD, 45);
        }
        else
        {
            motion.target = (int)((90.0f * PPR / 360.0f) * TURN_GAIN_R);
            motor_on(MOTOR_FORWARD, 45, MOTOR_REVERSE, 45);
        }

        send_ack();
    }
    else
    {
        motion.active = 0;
    }
}

void update_motion(void)
{
    if(motion.active)
    {
        if(motion.mode == MODE_AUTONOMOUS)
        {
            int dist = ultrasonic_read_cm();
            if(dist > 0 && dist < 15)
            {
                // Obstacle detected! Turn away.
                motor_on(MOTOR_REVERSE, 45, MOTOR_FORWARD, 45);
                delay_ms(400); // Wait for spin
                motor_on(MOTOR_FORWARD, 35, MOTOR_FORWARD, 35); // Resume forward
                motion.start_l = enc1_count;
                motion.start_r = enc2_count;
            }
            motion.timeout = 0; // Prevent stall timeout in autonomous mode
            return;
        }

        int dl = iabs_int(enc1_count - motion.start_l);
        int dr = iabs_int(enc2_count - motion.start_r);

        int progress = (dl + dr) / 2;

        if(progress == motion.last_progress)
            motion.timeout++;
        else
            motion.timeout = 0;

        motion.last_progress = progress;

        if(progress >= motion.target || motion.timeout > MOTION_TIMEOUT_TICKS)
        {
            motor_off();
            motion.active = 0;
            send_done();
        }
    }
}

/* ----------------------------------------------------------- */
/* MAIN                                                        */
/* ----------------------------------------------------------- */

int main(void)
{
    motor_init(M1A, M1B, M2A, M2B);
    ultrasonic_init(TRIG, ECHO);

    gpio_config(E1A, GPIO_INPUT);
    gpio_config(E1B, GPIO_INPUT);
    gpio_config(E2A, GPIO_INPUT);
    gpio_config(E2B, GPIO_INPUT);

    gpio_inten(E1A, 0, 1);
    gpio_inten(E2A, 1, 1);

    radio_hfclk_start();
    radio_init();
    radio_start_rx_radar();

    motion.last_progress = 0;

    while(1)
    {
        map_packet_t rx_pkt;
        command_t cmd_msg;

        radio_update_radar();

        if(radio_is_ready_radar())
        {
            radio_poll_map(&rx_pkt);

            if(rx_pkt.id == 2 || rx_pkt.id == 5)
            {
                cmd_msg.cmd = (char)rx_pkt.angle;
                cmd_msg.value = (rx_pkt.id == 5) ? (int)rx_pkt.distance : 0;
                start_motion(cmd_msg);
            }
        }

        update_motion();
        delay_ms(2);
    }
}

#elif defined(ROLE_LAPTOP)


#include "bsp.h"
#include "radio.h"
#include <stdio.h>

void encoder1_callback(void) { }
void encoder2_callback(void) { }

#define ACK_TIMEOUT_COUNT 2000
#define CMD_BUF_SIZE 8
#define HEARTBEAT_COUNT 3000

#ifndef UART_RX_PIN
#define UART_RX_PIN UART_RX
#endif

#ifndef UART_TX_PIN
#define UART_TX_PIN UART_TX
#endif
 
char cmd_buf[CMD_BUF_SIZE];
int  cmd_buf_len = 0;

void delay_ms(uint32_t ms)
{
    for(uint32_t i = 0; i < ms * 8000; i++);
}

int parse_uint(const char *s)
{
    int value = 0;

    while(*s >= '0' && *s <= '9')
    {
        value = (value * 10) + (*s - '0');
        s++;
    }

    return value;
}

int main(void)
{
    uart_init(UART_RX_PIN, UART_TX_PIN);
    printf("\r\n--- 2D Mapping Receiver Started ---\r\n");

    radio_hfclk_start();
    radio_init();
    radio_start_rx_radar();

    map_packet_t incoming;
    int waiting_for_ack = 0;
    uint32_t wait_counter = 0;
    uint32_t heartbeat_counter = 0;

    while(1)
    {
        /* ------------------------------ */
        /* 1. RADIO RX                   */
        /* ------------------------------ */
        radio_update_radar();

        if(radio_is_ready_radar())
        {
            radio_poll_map(&incoming);

            if(incoming.id == 3)
            {
                printf("ACK RECEIVED (L:%d R:%d)\r\n",
                        (int)incoming.encoder_left,
                        (int)incoming.encoder_right);
            }

            if(incoming.id == 4)
            {
                printf("DONE (L:%d R:%d)\r\n",
                        (int)incoming.encoder_left,
                        (int)incoming.encoder_right);

                waiting_for_ack = 0;
                wait_counter = 0;
            }
        }

        /* ------------------------------ */
        /* 2. TIMEOUT HANDLING           */
        /* ------------------------------ */
        if(waiting_for_ack)
        {
            wait_counter++;

            if(wait_counter > ACK_TIMEOUT_COUNT)
            {
                printf("TIMEOUT: ACK LOST -> RESETTING\r\n");
                waiting_for_ack = 0;
                wait_counter = 0;
            }
        }
        else
        {
            heartbeat_counter++;

            if(heartbeat_counter > HEARTBEAT_COUNT)
            {
                printf("READY\r\n");
                heartbeat_counter = 0;
            }
        }

        /* ------------------------------ */
        /* 3. UART -> RADIO TX           */
        /* ------------------------------ */
        if(uart_read_ready())
        {
            char c = uart_read_char();

            if(c == '\r') continue;

            if(c == '\n')
            {
                if(cmd_buf_len > 0)
                {
                    if(cmd_buf[0] >= 'a' && cmd_buf[0] <= 'z')
                        cmd_buf[0] -= 32;

                    char cmd = cmd_buf[0];

                    if(cmd=='F' || cmd=='B' || cmd=='L' || cmd=='R' || cmd=='S')
                    {
                        if(waiting_for_ack)
                        {
                            printf("BUSY (waiting ACK), dropped: %c\r\n", cmd);
                        }
                        else
                        {
                            map_packet_t pkt;
                            pkt.encoder_left  = 0;
                            pkt.encoder_right = 0;
                            pkt.reserved      = 0;

                            if(cmd_buf_len > 1)
                            {
                                cmd_buf[cmd_buf_len] = '\0';
                                int value = parse_uint(&cmd_buf[1]);

                                pkt.id       = 5;
                                pkt.angle    = (uint8_t)cmd;
                                pkt.distance = (uint16_t)value;

                                printf("PARAM CMD: %c val=%d\r\n", cmd, value);
                            }
                            else
                            {
                                pkt.id       = 2;
                                pkt.angle    = (uint8_t)cmd;
                                pkt.distance = 0;

                                printf("CMD SENT: %c\r\n", cmd);
                            }

                            delay_ms(2);
                            radio_tx_map(&pkt);

                            waiting_for_ack = 1;
                            wait_counter    = 0;
                        }
                    }
                    else
                    {
                        printf("INVALID: %c\r\n", cmd);
                    }

                    cmd_buf_len = 0;
                }
            }
            else
            {
                if(cmd_buf_len < CMD_BUF_SIZE - 1)
                    cmd_buf[cmd_buf_len++] = c;
            }
        }

        delay_ms(1);
    }
}




//#include "bsp.h"
//#include <stdio.h>

//void encoder1_callback(void) { }
//void encoder2_callback(void) { }

//int main(void)
//{
//    uart_init(UART_RX, UART_TX);
//    printf("BOOT OK\r\n");

//    while(1)
//    {
//        if(uart_read_ready())
//        {
//            char c = uart_read_char();
//            printf("GOT: %c\r\n", c);
//        }
//    }
//}




#else
#error "Define either ROLE_BOT or ROLE_LAPTOP"
#endif
