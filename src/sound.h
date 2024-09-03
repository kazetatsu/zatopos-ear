#ifndef _SOUND_H
#define _SOUND_H

#include <pico/sync.h>

#define NUM_MIC_CHS 6
#define SOUND_DEPTH 64
#define SOUND_BUF_LEN NUM_MIC_CHS * SOUND_DEPTH
#define SOUND_BUF_SIZE 2 * SOUND_BUF_LEN
#define SOUND_NUM_BUFS 3

extern uint8_t sound_front;
extern uint16_t *sound_bufs[SOUND_NUM_BUFS];
extern uint16_t sound_checker; // Increment at every writing sound_buf

void sound_shift_front(void);

#endif