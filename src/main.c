#include <stdio.h>
#include "pico/stdlib.h"

#include "mic.h"

#define SOUND_DEPTH 2

int main() {
    stdio_init_all();
    mic_init();

    uint8_t ch = 0;
    char cmd;
    uint16_t data[SOUND_DEPTH][CH_NUM];
    while(1) {
        scanf("%c", &cmd);

        mic_read(data, SOUND_DEPTH);

        for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
            printf("%02x,%03x", i, data[i][0]);
            for(uint8_t ch = 1; ch < CH_NUM; ++ch)
                printf(",%03x", data[i][ch]);
            printf("\n");
        }
    }
}