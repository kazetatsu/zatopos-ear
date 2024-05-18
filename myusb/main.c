#include <stdio.h>
#include "pico/stdlib.h"
#include "dev_lowlevel.h"

int main() {
    stdio_init_all();
    printf("USB Device Low-Level hardware example\n");
    usb_init();

    uint8_t data[2];
    data[0] = 0xca;
    data[1] = 0xfe;

    // Everything is interrupt driven so just loop here
    while (1) {
        sleep_ms(100);
        usb_send_data(data, 2);
    }

    return 0;
}
