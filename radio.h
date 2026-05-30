#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

/************************************************************
 * CLOCK
 ************************************************************/
void radio_hfclk_start(void);

/************************************************************
 * INITIALIZATION
 ************************************************************/
void radio_init(void);

/************************************************************
 * TELEMETRY PACKET (4 BYTES) - Original Version
 * ID | ANGLE | DIST_L | DIST_H
 ************************************************************/
void radio_tx_radar(uint8_t id, uint8_t angle, uint16_t distance);

void radio_start_rx_radar(void);
void radio_update_radar(void);
int  radio_is_ready_radar(void);
void radio_poll_radar(uint8_t *id, uint8_t *angle, uint16_t *distance);

/************************************************************
 * MAPPING PACKET (16 BYTES) - Extended Version
 * This version carries encoder data for 2D Point Clouds
 ************************************************************/

typedef struct {
    uint8_t  id;
    uint8_t  angle;
    uint16_t distance;
    int32_t  encoder_left;
    int32_t  encoder_right;
    uint32_t reserved; // Padding for 16-byte alignment
} map_packet_t;

void radio_tx_map(map_packet_t *data);
void radio_poll_map(map_packet_t *data);

#endif
