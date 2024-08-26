#include "sound.h"

critical_section_t crit_sec_sound_buf;
uint16_t sound_buf[SOUND_BUF_LEN];
uint16_t sound_check;

critical_section_t crit_sec_start_time;
uint32_t start_time;
