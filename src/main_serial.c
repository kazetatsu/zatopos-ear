#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include "sound.h"
#include "mic.h"

int main() {
    stdio_init_all();
    mic_init();

    while(1) {
        mic_swap_and_start();

        uint32_t data;
        for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
            data = mic_front_buffer[2 * i + 1];
            printf("%03x,", data & 0x3FF);
            printf("%03x,", (data >> 10) & 0x3FF);
            printf("%03x,", (data >> 20) & 0x3FF);

            data = mic_front_buffer[2 * i];
            printf("%03x,",  data & 0x3FF);
            printf("%03x,",  (data >> 10) & 0x3FF);
            printf("%03x\n", (data >> 20) & 0x3FF);
        }

        mic_wait_for_finish();
    }
}