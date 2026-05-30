#include "radio.h"
#include "nrf.h"
#include <string.h>

/************************************************************
 * PACKETS
 ************************************************************/

static uint8_t tx_packet[16];
static uint8_t rx_packet[16];

static volatile uint8_t rx_ready = 0;
static map_packet_t rx_map_data;

/************************************************************
 * CLOCK START
 ************************************************************/

void radio_hfclk_start(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (!NRF_CLOCK->EVENTS_HFCLKSTARTED);
}

/************************************************************
 * RADIO INIT
 ************************************************************/

void radio_init(void)
{
    NRF_RADIO->POWER = 1;

    NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_1Mbit;
    NRF_RADIO->FREQUENCY = 7;

    NRF_RADIO->TXPOWER =
        RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos;

    NRF_RADIO->PREFIX0 = 0xAA;
    NRF_RADIO->BASE0   = 0x11223344;

    NRF_RADIO->TXADDRESS = 0;
    NRF_RADIO->RXADDRESSES = 1;

    NRF_RADIO->PCNF0 =
        (0 << RADIO_PCNF0_LFLEN_Pos) |
        (0 << RADIO_PCNF0_S0LEN_Pos) |
        (0 << RADIO_PCNF0_S1LEN_Pos);

    NRF_RADIO->PCNF1 =
        (16 << RADIO_PCNF1_MAXLEN_Pos) |
        (16 << RADIO_PCNF1_STATLEN_Pos) |
        (3  << RADIO_PCNF1_BALEN_Pos) |
        (0  << RADIO_PCNF1_WHITEEN_Pos);

    NRF_RADIO->CRCCNF  = RADIO_CRCCNF_LEN_Two;
    NRF_RADIO->CRCINIT = 0xFFFF;
    NRF_RADIO->CRCPOLY = 0x11021;

    rx_ready = 0;
}

/************************************************************
 * RADIO STATE CONTROL
 ************************************************************/

static void radio_stop(void)
{
    NRF_RADIO->EVENTS_DISABLED = 0;

    NRF_RADIO->TASKS_DISABLE = 1;

    while (NRF_RADIO->EVENTS_DISABLED == 0);

    NRF_RADIO->EVENTS_DISABLED = 0;
}

static void radio_start_rx(void)
{
    rx_ready = 0;

    NRF_RADIO->PACKETPTR = (uint32_t)rx_packet;

    NRF_RADIO->EVENTS_END = 0;
    NRF_RADIO->EVENTS_READY = 0;

    NRF_RADIO->SHORTS =
        RADIO_SHORTS_READY_START_Msk |
        RADIO_SHORTS_END_START_Msk;

    NRF_RADIO->TASKS_RXEN = 1;
}

/************************************************************
 * TX MAP PACKET (FIXED)
 ************************************************************/

void radio_tx_map(map_packet_t *data)
{
    memcpy(tx_packet, data, 16);

    /* STOP RX cleanly */
    radio_stop();

    /* Disable shortcuts */
    NRF_RADIO->SHORTS = 0;

    NRF_RADIO->PACKETPTR = (uint32_t)tx_packet;

    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END   = 0;

    /* Enable TX */
    NRF_RADIO->TASKS_TXEN = 1;

    /* Wait READY */
    while (NRF_RADIO->EVENTS_READY == 0);
    NRF_RADIO->EVENTS_READY = 0;

    /* Start TX */
    NRF_RADIO->TASKS_START = 1;

    /* Wait END */
    while (NRF_RADIO->EVENTS_END == 0);
    NRF_RADIO->EVENTS_END = 0;

    /* Back to RX */
    radio_start_rx();
}

/************************************************************
 * TX RADAR PACKET (LEGACY)
 ************************************************************/

void radio_tx_radar(uint8_t id, uint8_t angle, uint16_t distance)
{
    memset(tx_packet, 0, 16);

    tx_packet[0] = id;
    tx_packet[1] = angle;
    tx_packet[2] = distance & 0xFF;
    tx_packet[3] = distance >> 8;

    radio_stop();

    NRF_RADIO->SHORTS = 0;

    NRF_RADIO->PACKETPTR = (uint32_t)tx_packet;

    NRF_RADIO->EVENTS_READY = 0;
    NRF_RADIO->EVENTS_END   = 0;

    NRF_RADIO->TASKS_TXEN = 1;

    while (NRF_RADIO->EVENTS_READY == 0);
    NRF_RADIO->EVENTS_READY = 0;

    NRF_RADIO->TASKS_START = 1;

    while (NRF_RADIO->EVENTS_END == 0);
    NRF_RADIO->EVENTS_END = 0;

    radio_start_rx();
}

/************************************************************
 * START RX
 ************************************************************/

void radio_start_rx_radar(void)
{
    radio_start_rx();
}

/************************************************************
 * UPDATE RX ENGINE
 ************************************************************/

void radio_update_radar(void)
{
    if (NRF_RADIO->EVENTS_END)
    {
        NRF_RADIO->EVENTS_END = 0;

        if (NRF_RADIO->CRCSTATUS)
        {
            memcpy(&rx_map_data, rx_packet, 16);
            rx_ready = 1;
        }
    }
}

/************************************************************
 * CHECK READY
 ************************************************************/

int radio_is_ready_radar(void)
{
    return rx_ready;
}

/************************************************************
 * READ MAP PACKET
 ************************************************************/

void radio_poll_map(map_packet_t *data)
{
    rx_ready = 0;
    memcpy(data, &rx_map_data, 16);
}