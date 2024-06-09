#include <stdio.h>
#include "pico/stdlib.h"

#include "mic.h"

int main() {
    stdio_init_all();

    char cmd;
    scanf("%c", &cmd);

    printf("initting\n");

    mic_init();

    printf("ready\n");

    uint8_t ch = 0;
    while(1) {
        scanf("%c", &cmd);

        uint16_t data = mic_read(ch);

        printf("%04u\n", data);

        // print_pio_status();
    }
}