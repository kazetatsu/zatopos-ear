#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "mcp3008.pio.h"
#include "mic.h"

static PIO pio;
static uint sm;
static pio_sm_config cfg;
static uint offset;
static uint32_t buf[2 * SOUND_DEPTH];
static int dma_ch;

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

void mic_start() {
    dma_ch = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_ch);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));

    dma_channel_configure(
        dma_ch,           // Channel to be configured
        &c,               // configuration
        buf,              // initial write address
        &pio->rxf[sm],    // initial read address
        2 * SOUND_DEPTH,  // number of transfers
        false             // true <=> start immediately
    );

    mic_stop();
    dma_channel_start(dma_ch);
    mic_refresh();
}

void mic_get_sound_blocking(uint16_t sound[SOUND_DEPTH][CH_NUM]) {
    dma_channel_wait_for_finish_blocking(dma_ch);

    mic_stop();

    dma_channel_cleanup(dma_ch);
    dma_channel_unclaim(dma_ch);

    uint32_t b;
    for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
        b = buf[2 * i];
        sound[i][5] = (b >> 20) & 0x3FF;
        sound[i][4] = (b >> 10) & 0x3FF;
        sound[i][3] = b & 0x3FF;
        b = buf[2 * i + 1];
        sound[i][2] = (b >> 20) & 0x3FF;
        sound[i][1] = (b >> 10) & 0x3FF;
        sound[i][0] = b & 0x3FF;
    }
}
