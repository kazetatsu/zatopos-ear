#ifndef _MIC_H
#define _MIC_H

#include <pico/types.h>

#define PIN_MOSI  16 // to   Pico from MCP3008
#define PIN_CS    17
#define PIN_SCLK  18
#define PIN_MISO  19 // from Pico to   MCP3008

// #define PIN_MOSI  3 // to   xiao from MCP3008
// #define PIN_CS    1
// #define PIN_SCLK  2
// #define PIN_MISO  4 // from xiao to   MCP3008

void mic_init();
void mic_swap_and_start();
void mic_wait_for_finish();
void mic_fill_sound_buf(void);
// void mic_get_sound(uint16_t sound[SOUND_DEPTH][CH_NUM]);

extern uint32_t *mic_front_buffer;
extern uint32_t *mic_back_buffer;

#endif