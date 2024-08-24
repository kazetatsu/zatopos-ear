#ifndef _SOUND_H
#define _SOUND_H

#include <pico/sync.h>

#define NUM_MIC_CHS 6
#define SOUND_DEPTH 64
#define SOUND_BUF_LEN NUM_MIC_CHS * SOUND_DEPTH
#define SOUND_BUF_SIZE 2 * SOUND_BUF_LEN

extern critical_section_t crit_sec_sound_buf;
extern uint16_t sound_buf[SOUND_BUF_LEN];

extern critical_section_t crit_sec_start_time;
extern uint32_t start_time;

#endif