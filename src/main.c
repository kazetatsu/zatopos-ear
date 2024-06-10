#include <stdio.h>
#include "pico/stdlib.h"

#include "mic.h"

int main() {
    stdio_init_all();
    mic_init();

    char cmd;
    uint16_t sound[SOUND_DEPTH][CH_NUM];
    while(1) {
        scanf("%c", &cmd);

        mic_start();
        mic_get_sound_blocking(sound);

        for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
            printf("%02x,%03x", i, sound[i][0]);
            for(uint8_t ch = 1; ch < CH_NUM; ++ch)
                printf(",%03x", sound[i][ch]);
            printf("\n");
        }
    }
}