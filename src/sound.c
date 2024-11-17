#include "sound.h"

static uint16_t sound_buf_a[SOUND_BUF_LEN];
static uint16_t sound_buf_b[SOUND_BUF_LEN];
static uint16_t sound_buf_c[SOUND_BUF_LEN];
static uint16_t sound_buf_d[SOUND_BUF_LEN];

uint8_t sound_front = 0;
uint16_t *sound_bufs[SOUND_NUM_BUFS] = {
    sound_buf_a,
    sound_buf_b,
    sound_buf_c,
    sound_buf_d
};
uint16_t sound_checker = 0;

void sound_shift_front(void) {
    uint8_t temp = sound_front;
    temp++;
    temp &= 0b11;
    sound_front = temp;
    sound_checker++;
}
