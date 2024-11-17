#ifndef _MIC_H
#define _MIC_H

#include <pico/types.h>
#include "sound.h"

#define PIN_CS      9
#define PIN_SCLK   10
#define PIN_MISO   11 // from Pico to   MCP3002
#define PIN_MOSI0  12 // to   Pico from MCP3002
#define PIN_MOSI1  13
#define PIN_MOSI2  14

#define MIC_BUF_LEN 2 * SOUND_DEPTH

void mic_init();
void mic_swap_and_start();
void mic_wait_for_finish();
void mic_fill_sound_buf(void);

extern uint32_t *mic_front_buffer;
extern uint32_t *mic_back_buffer;

#endif