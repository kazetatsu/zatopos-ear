#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/irq.h"

#include "dev_lowlevel.h"

uint32_t t;
static mutex_t mtx;

static inline void my_mutex_enter(mutex_t *mtx) {
    uint32_t owner;
    if(!mutex_try_enter(mtx, &owner)) {
        if(owner == get_core_num()) return;
        mutex_enter_blocking(mtx);
    }
}

static uint8_t wbuf[4];
static inline void set_data_to_buffer(uint32_t t) {
    wbuf[3] = t;
    wbuf[2] = t >>  8;
    wbuf[1] = t >> 16;
    wbuf[0] = t >> 24;
}

void usb_callback(uint8_t *buffer, uint16_t length) {
    if((char)buffer[0] == 'r') {
        my_mutex_enter(&mtx);
        set_data_to_buffer(t);
        mutex_exit(&mtx);
        usb_send_data(wbuf, 4);
    }
}

// main function of core1
void core1_entry() {
    while(1) {
        sleep_ms(1000);
        my_mutex_enter(&mtx);
        t = time_us_32();
        mutex_exit(&mtx);
    }
}

inline void set_buffer(uint8_t *buf, uint32_t data) {
    buf[3] = data;
    buf[2] = data >>  8;
    buf[1] = data >> 16;
    buf[0] = data >> 24;
}

int main() {
    stdio_init_all();
    usb_init();

    mutex_init(&mtx);

    multicore_launch_core1(core1_entry);

    // Everything is interrupt driven so just loop here
    while (1) {
        tight_loop_contents();
    }

    return 0;
}
