#ifndef _MIC_H
#define _MIC_H

#include "pico/types.h"

#define PIN_MOSI  16 // to   Pico from MCP3008
#define PIN_CS    17
#define PIN_SCLK  18
#define PIN_MISO  19 // from Pico to   MCP3008

void mic_init();
uint16_t mic_read(uint8_t ch);

void print_pio_status();

#endif