#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#include "mic.h"

int main() {
    stdio_init_all();
    mic_init();

    while(1) {
        mic_swap_and_start();
        uint32_t t = time_us_32();
        printf("time:%08x\n", t);

        uint32_t b;
        for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
            b = mic_front_buffer[2 * i + 1];
            printf("%03x,", b & 0x3FF);
            printf("%03x,", (b >> 10) & 0x3FF);
            printf("%03x,", (b >> 20) & 0x3FF);

            b = mic_front_buffer[2 * i];
            printf("%03x,",  b & 0x3FF);
            printf("%03x,",  (b >> 10) & 0x3FF);
            printf("%03x\n", (b >> 20) & 0x3FF);
        }

        mic_wait_for_finish();
    }
}
