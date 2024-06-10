#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "mcp3008.pio.h"
#include "mic.h"

static PIO pio;
static uint sm;
static pio_sm_config cfg;
static uint offset;

static inline void mic_refresh() {
    pio_sm_init(pio, sm, offset, &cfg);
    pio_sm_set_enabled(pio, sm, true);
}

static inline void mic_stop() {
    pio_sm_set_enabled(pio, sm, false);
}

void mic_init() {
    pio = pio0;
    offset = pio_add_program(pio, &mcp3008_program);
    sm = pio_claim_unused_sm(pio, true);
    cfg = mcp3008_program_get_default_config(offset);

    sm_config_set_clkdiv(&cfg, 16.0f);

    // CS & SCLK
    sm_config_set_sideset_pins(&cfg, PIN_CS);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_CS, 2, true);
    pio_gpio_init(pio, PIN_SCLK);
    pio_gpio_init(pio, PIN_CS);

    // MISO = from pico to adc
    sm_config_set_out_pins(&cfg, PIN_MISO, 1);
    sm_config_set_out_shift(&cfg, true, false, 32);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_MISO, 1, true);
    sm_config_set_set_pins(&cfg, PIN_MISO, 1);
    pio_gpio_init(pio, PIN_MISO);

    // MOSI = from adc to pico
    sm_config_set_in_pins(&cfg, PIN_MOSI);
    sm_config_set_in_shift(&cfg, false, true, 30);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_MOSI, 1, false);
    pio_gpio_init(pio, PIN_MOSI);

    mic_refresh();
}

void mic_read(uint16_t data[][CH_NUM], uint8_t depth) {
    mic_refresh();

    uint32_t rbuf;

    for(uint8_t i = 0; i < depth; ++i) {
        rbuf = pio_sm_get_blocking(pio, sm);
        data[i][0] = (rbuf >> 20) & 0x03FF;
        data[i][1] = (rbuf >> 10) & 0x03FF;
        data[i][2] = rbuf & 0x03FF;

        rbuf = pio_sm_get_blocking(pio, sm);
        data[i][3] = (rbuf >> 20) & 0x03FF;
        data[i][4] = (rbuf >> 10) & 0x03FF;
        data[i][5] = rbuf & 0x03FF;
    }

    mic_stop();
}
