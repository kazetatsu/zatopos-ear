#include <stdio.h>

#include "hardware/pio.h"

#include "mcp3008.pio.h"
#include "mic.h"

static PIO pio;
static uint sm;

void mic_init() {
    pio = pio0;
    uint offset = pio_add_program(pio, &mcp3008_program);
    sm = pio_claim_unused_sm(pio, true);

    pio_sm_config cfg = mcp3008_program_get_default_config(offset);

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
    sm_config_set_in_shift(&cfg, false, true, 10);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_MOSI, 1, false);
    pio_gpio_init(pio, PIN_MOSI);

    pio_sm_init(pio, sm, offset, &cfg);
    pio_sm_set_enabled(pio, sm, true);
}

uint16_t mic_read(uint8_t ch) {
    // printf("preparing ch %u\n", ch);
    uint32_t tbuf = 0b111 & ch;
    pio_sm_put_blocking(pio, sm, tbuf);
    // printf("B = %08x\n", tbuf);
    uint32_t rbuf = pio_sm_get_blocking(pio, sm);
    // printf("complete\n");
    // printf("D = %08x\n", rbuf);
    return 0b1111111111 & rbuf;
}

void print_pio_status() {
    if(pio == pio0)
    printf("pio          : pio0\n");
    else
    printf("pio          : pio1\n");
    printf("sm           : %u\n", sm);
    printf("RX_FIFO level: %u\n", pio_sm_get_rx_fifo_level(pio, sm));
    printf("TX_FIFO level: %u\n", pio_sm_get_rx_fifo_level(pio, sm));
}
