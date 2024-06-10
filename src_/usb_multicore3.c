#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/irq.h"

#include "dev_lowlevel.h"
#include "micarr.h"

static mutex_t mtx_sound;

uint16_t sound[CH_NUM][SOUND_LEN];

#define TX_BUF_LEN  2 * CH_NUM * SOUND_LEN // must be multiple of 64
uint8_t tx_buf[TX_BUF_LEN]; // buffer of USB packet from device

inline void data_to_write_buffer() {
    for(uint16_t ch = 0; ch < CH_NUM; ++ch) {
        for(uint16_t k = 0; k < SOUND_LEN; ++k) {
            tx_buf[ch * SOUND_LEN * 2 + k * 2  + 0] = sound[ch][k];
            tx_buf[ch * SOUND_LEN * 2 + k * 2  + 1] = sound[ch][k] >> 8;
        }
    }
}

static inline void my_mutex_enter(mutex_t *mtx) {
    uint32_t owner;
    if(!mutex_try_enter(mtx, &owner)) {
        if(owner == get_core_num()) return;
        mutex_enter_blocking(mtx);
    }
}

inline bool mutex_try_enter_core0(mutex_t *mtx) {
    uint32_t owner;
    return mutex_try_enter(mtx, &owner) || owner == get_core_num();
}

void usb_callback(uint8_t *buffer, uint16_t length) {
    if((char)buffer[0] == 'r') {
        my_mutex_enter(&mtx_sound);
        data_to_write_buffer();
        mutex_exit(&mtx_sound);
        for(uint8_t i = 0; i < TX_BUF_LEN; i += 64) {
            usb_send_data(tx_buf + i, 64);
        }
    }
}

// main function of core1
void core1_entry() {
    micarr_init();
    while(1) {
        micarr_run();
        my_mutex_enter(&mtx_sound);
        micarr_read(sound);
        mutex_exit(&mtx_sound);
    }
}

int main() {
    stdio_init_all();
    usb_init();

    mutex_init(&mtx_sound);

    multicore_launch_core1(core1_entry);

    // Everything is interrupt driven so just loop here
    while (1) {
        tight_loop_contents();
    }

    return 0;
}
