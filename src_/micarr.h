#include "pico/types.h"

#define CH_NUM 1
#define SOUND_LEN 64

void micarr_init();
void micarr_run();
void micarr_read(uint16_t buffer[][SOUND_LEN]);
