#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/irq.h"

#include "micarr.h"

static mutex_t mtx_sound;

uint16_t sound[CH_NUM][SOUND_LEN];
bool sound_is_latest = false;

#define TX_BUF_LEN  2 * SOUND_LEN
char tx_bufs[CH_NUM][TX_BUF_LEN + 1]; // buffer of serial transmission from pico

inline void data_to_write_buffer() {
    for(uint16_t ch = 0; ch < CH_NUM; ++ch) {
        for(uint16_t k = 0; k < SOUND_LEN; ++k) {
            tx_bufs[ch][k * 2  + 0] = sound[ch][k];
            tx_bufs[ch][k * 2  + 1] = sound[ch][k] >> 8;
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

// inline bool mutex_try_enter_core0(mutex_t *mtx) {
//     uint32_t owner;
//     return mutex_try_enter(mtx, &owner) || owner == get_core_num();
// }

// main function of core1
void core1_entry() {
    micarr_init();
    while(1) {
        micarr_run();
        my_mutex_enter(&mtx_sound);
        micarr_read(sound);
        sound_is_latest = true;
        mutex_exit(&mtx_sound);
    }
}

int main() {
    stdio_init_all();

    mutex_init(&mtx_sound);

    multicore_launch_core1(core1_entry);

    char cmd;
    while (1) {
        scanf("%c", &cmd);

        if(cmd == 'r') {
            my_mutex_enter(&mtx_sound);
            while(!sound_is_latest) {
                mutex_exit(&mtx_sound);
                sleep_ms(3);
                my_mutex_enter(&mtx_sound);
            }
            data_to_write_buffer();
            sound_is_latest = false;
            mutex_exit(&mtx_sound);
            for(uint8_t ch = 0; ch < CH_NUM; ++ch) {
                for(uint8_t i = 0; i < TX_BUF_LEN; ++i) {
                    printf("%c", tx_bufs[ch][i]);
                }
                printf("\n");
            }
        }
        else printf("what?\n");
    }

    return 0;
}
