// based on
// https://www.denshi.club/parts/2021/04/raspberry-pi-pico-12-spi-apia-dmcp.html

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/timer.h"

#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS   17

#define SPI_PORT spi0

#define DATASET_LEN 8
#define CH_NUM 2

const uint8_t ch_select[] = {0b10000000, 0b10010000, 0b10100000, 0b10110000, 0b11000000, 0b11010000, 0b11100000, 0b11110000};

int dataset_a[CH_NUM * DATASET_LEN];
int dataset_b[CH_NUM * DATASET_LEN];
int dataset_c[CH_NUM * DATASET_LEN];

static mutex_t mutex_a;
static mutex_t mutex_b;
static mutex_t mutex_c;

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}


static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}


void setup_SPI() {
    // use SPI0
    // baudrate = 1.5MHz
    spi_init(SPI_PORT, 1500 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
}


static inline void my_mutex_enter(mutex_t *mtx) {
    uint32_t owner;
    if(!mutex_try_enter(mtx, &owner)) {
        if(owner == get_core_num()) return;
        mutex_enter_blocking(mtx);
    }
}


// return dataset
static inline int* enter_dataset(char dataset_code) {
    switch(dataset_code) {
        case 'a':
            my_mutex_enter(&mutex_a);
            return dataset_a;
        case 'b':
            my_mutex_enter(&mutex_b);
            return dataset_b;
        case 'c':
            my_mutex_enter(&mutex_c);
            return dataset_c;
        default:
            return NULL;
    }
}


// return next dataset code
static inline char exit_dataset(char dataset_code) {
    switch(dataset_code) {
        case 'a':
            mutex_exit(&mutex_a);
            return 'b';
        case 'b':
            mutex_exit(&mutex_b);
            return 'c';
        case 'c':
            mutex_exit(&mutex_c);
            return 'a';
        default:
            return '-';
    }
}


static inline int read_adc(uint8_t ch) {
    uint8_t write_bytes[] = {0b00000001, 0, 0b00000000};
    uint8_t read_bytes[3];

    write_bytes[1] = ch_select[ch];
    cs_select();
    spi_write_read_blocking(SPI_PORT, write_bytes, read_bytes, 3);
    cs_deselect();

    return (read_bytes[1] & 0b00000011) << 8 | read_bytes[2];
}


// main function of core1
void core1_entry() {
    int* wdataset; // writing datset
    char wdataset_code = 'a';

    uint32_t start_time, finish_time;

    while(1) {
        // mutex lock
        // block until core1 is allowed to access dataset
        wdataset = enter_dataset(wdataset_code);

        start_time = time_us_32();

        // communicate with A/D converter (MCP3008)
        // measure sound
        for(uint16_t i = 0; i < DATASET_LEN; ++i) {
            for(uint8_t ch = 0; ch < CH_NUM; ++ch) {
                wdataset[CH_NUM * i + ch] = read_adc(ch);
            }
            sleep_ms(250);
        }

        finish_time = time_us_32();

        multicore_fifo_push_blocking(start_time);
        multicore_fifo_push_blocking(finish_time);

        // mutex unlock
        wdataset_code = exit_dataset(wdataset_code);
    }
}


int main() {
    stdio_init_all();

    setup_SPI();

    mutex_init(&mutex_a);
    mutex_init(&mutex_b);
    mutex_init(&mutex_c);

    char cmd;
    scanf("%c", &cmd);

    multicore_launch_core1(core1_entry);

    int* rdataset1;
    int* rdataset2;
    char rdataset1_code = 'a';
    char rdataset2_code = 'b';
    char temp;

    uint32_t stime1, stime2, ftime1, ftime2; // start time, finish time

    stime1 = multicore_fifo_pop_blocking();
    ftime1 = multicore_fifo_pop_blocking();

    rdataset1 = enter_dataset(rdataset1_code);

    stime2 = multicore_fifo_pop_blocking();
    ftime2 = multicore_fifo_pop_blocking();

    rdataset2 = enter_dataset(rdataset2_code);

    while(1) {
        // dummy
        printf("t:%d,%d\n", stime1, ftime1);
        for(uint16_t i =0; i < DATASET_LEN; ++i) {
            printf("d:%04d,%04d\n", rdataset1[CH_NUM * i + 0], rdataset1[CH_NUM * i + 1]);
        }
        sleep_ms(1000);

        // swap
        temp = rdataset1_code;
        rdataset1_code = rdataset2_code;
        stime1 = stime2;
        ftime1 = ftime2;
        rdataset1 = rdataset2;
        rdataset2_code = exit_dataset(temp);
        stime2 = multicore_fifo_pop_blocking();
        ftime2 = multicore_fifo_pop_blocking();
        rdataset2 = enter_dataset(rdataset2_code);
    }

    return 0;
}
