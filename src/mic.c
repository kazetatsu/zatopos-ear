#include <string.h>

#include <hardware/dma.h>
#include <hardware/gpio.h>
#include <hardware/pio.h>

#include <pico/stdlib.h>
#include <pico/sync.h>

#include "mcp3008.pio.h"
// #include "mcp3008_6ch_full.pio.h"
#include "mic.h"
#include "sound.h"

static PIO pio;
static uint pio_sm;
static pio_sm_config pio_cfg;
static uint pio_offset;

uint32_t *mic_front_buffer;
uint32_t *mic_back_buffer;

static uint32_t buf_a[SOUND_BUF_LEN];
static uint32_t buf_b[SOUND_BUF_LEN];

static uint16_t sound_buf_copy_src[SOUND_BUF_LEN];

static bool buf_a_is_front;

static int dma_ch;
static dma_channel_config dma_cfg;

void mic_init() {
    /**
     * PIO config
     */
    pio = pio0;
    pio_offset = pio_add_program(pio, &mcp3008_program);
    pio_sm = pio_claim_unused_sm(pio, true);
    pio_cfg = mcp3008_program_get_default_config(pio_offset);

    sm_config_set_clkdiv(&pio_cfg, 16.0f);

    // CS & SCLK
    sm_config_set_sideset_pins(&pio_cfg, PIN_CS);
    pio_sm_set_consecutive_pindirs(pio, pio_sm, PIN_CS, 2, true);
    pio_gpio_init(pio, PIN_SCLK);
    pio_gpio_init(pio, PIN_CS);

    // MISO = from pico to adc
    sm_config_set_out_pins(&pio_cfg, PIN_MISO, 1);
    sm_config_set_out_shift(&pio_cfg, true, false, 32);
    pio_sm_set_consecutive_pindirs(pio, pio_sm, PIN_MISO, 1, true);
    sm_config_set_set_pins(&pio_cfg, PIN_MISO, 1);
    pio_gpio_init(pio, PIN_MISO);

    // MOSI = from adc to pico
    sm_config_set_in_pins(&pio_cfg, PIN_MOSI);
    sm_config_set_in_shift(&pio_cfg, false, true, 30);
    pio_sm_set_consecutive_pindirs(pio, pio_sm, PIN_MOSI, 1, false);
    pio_gpio_init(pio, PIN_MOSI);

    pio_sm_init(pio, pio_sm, pio_offset, &pio_cfg);
    pio_sm_set_enabled(pio, pio_sm, false); // stop untill mic_start() is called

    /**
     * DMA config
     */
    dma_ch = dma_claim_unused_channel(true);
    dma_cfg = dma_channel_get_default_config(dma_ch);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(pio, pio_sm, false));

    buf_a_is_front = true;
    // set dummy data
    for(uint8_t i = 0; i < 2 * SOUND_DEPTH; ++i) {
        buf_b[i] = 0;
    }
}

void mic_swap_and_start() {
    if(buf_a_is_front) {
        // critical_section_enter_blocking(&crit_sec_sound_buf);
        mic_front_buffer = buf_b;
        // critical_section_exit(&crit_sec_sound_buf);
        mic_back_buffer  = buf_a;
        buf_a_is_front = false;
    } else {
        // critical_section_enter_blocking(&crit_sec_sound_buf);
        mic_front_buffer = buf_a;
        // critical_section_exit(&crit_sec_sound_buf);
        mic_back_buffer  = buf_b;
        buf_a_is_front = true;
    }

    dma_channel_configure(
        dma_ch,            // Channel to be configured
        &dma_cfg,          // configuration
        mic_back_buffer,   // initial write address
        &pio->rxf[pio_sm], // initial read address
        2 * SOUND_DEPTH,   // number of transfers
        false              // true <=> start immediately
    );

    dma_channel_start(dma_ch);

    pio_sm_clear_fifos(pio, pio_sm);
    pio_sm_restart(pio, pio_sm);
    pio_sm_set_enabled(pio, pio_sm, true);
}

void mic_wait_for_finish() {
    dma_channel_wait_for_finish_blocking(dma_ch);
    pio_sm_set_enabled(pio, pio_sm, false);
}

void mic_fill_sound_buf(void) {
    uint16_t r;
    uint32_t data;

    for (uint16_t i = 0; i < SOUND_DEPTH; i++) {
        r = NUM_MIC_CHS * i;

        data = mic_front_buffer[2 * i];
        sound_buf_copy_src[r + 0] = data & 0x3ff;
        data >>= 10;
        sound_buf_copy_src[r + 1] = data & 0x3ff;
        data >>= 10;
        sound_buf_copy_src[r + 2] = data & 0x3ff;

        data = mic_front_buffer[2 * i + 1];
        sound_buf_copy_src[r + 3] = data & 0x3ff;
        data >>= 10;
        sound_buf_copy_src[r + 4] = data & 0x3ff;
        data >>= 10;
        sound_buf_copy_src[r + 5] = data & 0x3ff;
    }

    critical_section_enter_blocking(&crit_sec_sound_buf);
    memcpy(sound_buf, sound_buf_copy_src, sizeof(uint16_t) * SOUND_BUF_LEN);
    critical_section_exit(&crit_sec_sound_buf);
}

// void mic_get_sound(uint16_t sound[SOUND_DEPTH][CH_NUM]) {
//     dma_channel_cleanup(dma_ch);
//     dma_channel_unclaim(dma_ch);

//     uint32_t b;
//     for(uint8_t i = 0; i < SOUND_DEPTH; ++i) {
//         b = mic_front_buffer[2 * i];
//         sound[i][5] = (b >> 20) & 0x3FF;
//         sound[i][4] = (b >> 10) & 0x3FF;
//         sound[i][3] = b & 0x3FF;
//         b = mic_front_buffer[2 * i + 1];
//         sound[i][2] = (b >> 20) & 0x3FF;
//         sound[i][1] = (b >> 10) & 0x3FF;
//         sound[i][0] = b & 0x3FF;
//     }
// }
