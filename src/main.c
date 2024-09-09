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

        mic_fill_sound_buf();
        sound_shift_front();

        mic_wait_for_finish();
    }
}

int main() {
    stdio_init_all();

    multicore_launch_core1(core1_entry);

    usb_device_init();

    while (1) {
        tight_loop_contents();
    }
}
