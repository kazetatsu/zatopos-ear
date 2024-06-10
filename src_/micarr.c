#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
// #include "hardware/dma.h"

#include "micarr.h"

#define RAW_SOUND_LEN CH_NUM * SOUND_LEN

static uint16_t raw_sound[RAW_SOUND_LEN];
// static uint8_t sound_raw[CH_NUM * SOUND_LEN];
// static uint dma_chan;

void micarr_init() {
    gpio_init(26);
    adc_init();
    // adc_select_input(CAPTURE_CHANNEL);
    uint adc_in_mask = 0;
    for(uint8_t ch = 0; ch < CH_NUM; ++ch)
        adc_in_mask |= 1 << ch;
    adc_set_round_robin(adc_in_mask);
    // adc_fifo_setup(
    //     true,    // Write each completed conversion to the sample FIFO
    //     true,    // Enable DMA data request (DREQ)
    //     1,       // DREQ (and IRQ) asserted when at least 1 sample present
    //     false,   // We won't see the ERR bit because of 8 bit reads; disable.
    //     true     // Shift each sample to 8 bits when pushing to FIFO
    // );

    // // Divisor of 0 -> full speed. Free-running capture with the divider is
    // // equivalent to pressing the ADC_CS_START_ONCE button once per `div + 1`
    // // cycles (div not necessarily an integer). Each conversion takes 96
    // // cycles, so in general you want a divider of 0 (hold down the button
    // // continuously) or > 95 (take samples less frequently than 96 cycle
    // // intervals). This is all timed by the 48 MHz ADC clock.
    // adc_set_clkdiv(0);

    // // Set up the DMA to start transferring data as soon as it appears in FIFO
    // dma_chan = dma_claim_unused_channel(true);
    // dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    // // Reading from constant address, writing to incrementing byte addresses
    // channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    // channel_config_set_read_increment(&cfg, false);
    // channel_config_set_write_increment(&cfg, true);

    // // Pace transfers based on availability of ADC samples
    // channel_config_set_dreq(&cfg, DREQ_ADC);

    // dma_channel_configure(dma_chan, &cfg,
    //     sound_raw,               // dst
    //     &adc_hw->fifo,       // src
    //     CH_NUM * SOUND_LEN, // transfer count
    //     false                // start immediately
    // );
}

void micarr_run() {
    // printf("Starting capture\n");
    // dma_channel_start(dma_chan);
    // adc_run(true);

    // // Once DMA finishes, stop any new conversions from starting, and clean up
    // // the FIFO in case the ADC was still mid-conversion.
    // dma_channel_wait_for_finish_blocking(dma_chan);
    // printf("Capture finished\n");

    // adc_run(false);
    // dma_channel_abort(dma_chan);
    // adc_fifo_drain();
    for(uint8_t i = 0; i < RAW_SOUND_LEN; ++i) {
        raw_sound[i] = adc_read();
        sleep_ms(1);
    }
}

void micarr_read(uint16_t buffer[][SOUND_LEN]) {
    for(uint8_t ch = 0; ch < CH_NUM; ++ch) {
        for(uint8_t k = 0; k < SOUND_LEN; ++k) {
            buffer[ch][k] = raw_sound[k * CH_NUM + ch];
        }
    }
}
