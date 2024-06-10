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
    usb_init();

    mutex_init(&mtx);

    multicore_launch_core1(core1_entry);

    uint8_t data[6];
    data[0] = 0xca;
    data[1] = 0xfe;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    data[5] = 0;

    // Everything is interrupt driven so just loop here
    while (1) {
        sleep_ms(100);
        my_mutex_enter(&mtx);
        set_buffer(data + 2, t);
        usb_send_data(data, 6);
        mutex_exit(&mtx);
    }

    return 0;
}
