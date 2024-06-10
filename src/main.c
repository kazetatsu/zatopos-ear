#include <stdio.h>
#include "pico/stdlib.h"

#include "mic.h"

int main() {
    stdio_init_all();
    mic_init();

    uint8_t ch = 0;
    char cmd;
    uint16_t data[CH_NUM];
    while(1) {
        scanf("%c", &cmd);

        mic_read(data);

        printf("%04x", data[0]);
        for(uint8_t ch = 1; ch < CH_NUM; ++ch)
            printf(",%04x", data[ch]);
        printf("\n");
    }
}