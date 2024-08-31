#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#include "mic.h"
#include "usb/device.h"

void core1_entry() {
    mic_init();

    while(1) {
        mic_swap_and_start();

        uint32_t st = time_us_32();
        critical_section_enter_blocking(&crit_sec_start_time);
        start_time = st;
        critical_section_exit(&crit_sec_start_time);

        mic_fill_sound_buf();

        mic_wait_for_finish();
    }
}

int main() {
    stdio_init_all();

    critical_section_init(&crit_sec_sound_buf);
    critical_section_init(&crit_sec_start_time);
    multicore_launch_core1(core1_entry);

    usb_device_init();

    while (1) {
        tight_loop_contents();
    }
}
