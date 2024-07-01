#ifndef _MIC_H
#define _MIC_H

#include "pico/types.h"

// #define PIN_MOSI  16 // to   Pico from MCP3008
// #define PIN_CS    17
// #define PIN_SCLK  18
// #define PIN_MISO  19 // from Pico to   MCP3008

#define PIN_MOSI  3 // to   xiao from MCP3008
#define PIN_CS    1
#define PIN_SCLK  2
#define PIN_MISO  4 // from xiao to   MCP3008

#define CH_NUM 6
#define SOUND_DEPTH 64

void mic_init();
void mic_read(uint16_t data[][CH_NUM], uint8_t depth);
void mic_start();
void mic_get_sound_blocking(uint16_t sound[SOUND_DEPTH][CH_NUM]);

#endif