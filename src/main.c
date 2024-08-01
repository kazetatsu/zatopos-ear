#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#include "mic.h"

critical_section_t crit_sec_sound_buf;

void core1_entry() {
    mic_init();

    while(1) {
        mic_swap_and_start();

        uint32_t start_time = time_us_32();
        multicore_fifo_push_blocking(start_time);

        mic_wait_for_finish();
    }
}

int main() {
    critical_section_init(&crit_sec_sound_buf);
    multicore_fifo_drain();
    multicore_launch_core1(core1_entry);

    stdio_init_all();
    uint32_t printf_buffer[2 * SOUND_DEPTH];

    uint32_t start_time =  multicore_fifo_pop_blocking();

    while(1) {
        uint32_t next_start_time = multicore_fifo_pop_blocking();

        critical_section_enter_blocking(&crit_sec_sound_buf);
        for(uint8_t i = 2 * SOUND_DEPTH - 1; i < 255; --i) {
            printf_buffer[i] = mic_front_buffer[i];
        }
        critical_section_exit(&crit_sec_sound_buf);

        uint32_t b;
        for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
            b = printf_buffer[2 * i + 1];
            printf("%03x,", b & 0x3FF);
            printf("%03x,", (b >> 10) & 0x3FF);
            printf("%03x,", (b >> 20) & 0x3FF);

            b = printf_buffer[2 * i];
            printf("%03x,",  b & 0x3FF);
            printf("%03x,",  (b >> 10) & 0x3FF);
            printf("%03x\n", (b >> 20) & 0x3FF);
        }

        start_time = next_start_time;
    }
}
