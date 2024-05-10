// based on
// https://www.denshi.club/parts/2021/04/raspberry-pi-pico-12-spi-apia-dmcp.html

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_MISO 16
#define PIN_CS   17

#define SPI_PORT spi0

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

void setup_SPI(){
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

int readADC(uint8_t ch){
    uint8_t writeData[] = {0b00000001, 0x00, 0x00};
    switch(ch){
        case 0:
            writeData[1] = 0b10000000;
            break;
        case 1:
            writeData[1] = 0b10010000;
            break;
        case 2:
            writeData[1] = 0b10100000;
            break;
        case 3:
            writeData[1] = 0b10110000;
            break;
        case 4:
            writeData[1] = 0b11000000;
            break;
        case 5:
            writeData[1] = 0b11010000;
            break;
        case 6:
            writeData[1] = 0b11100000;
            break;
        case 7:
            writeData[1] = 0b11110000;
            break;
        default:
            return -1;
    }
    uint8_t buffer[3];

    cs_select();
    spi_write_read_blocking(SPI_PORT, writeData, buffer, 3);
    cs_deselect();

    return (buffer[1] & 0b00000011) << 8 | buffer[2];
}

int main() {
    stdio_init_all();

    setup_SPI();

    char c;
    uint8_t ch;
    while(1){
        scanf("%c", &c);
        ch = (uint8_t)(c - '0');

        printf("ch%u: %d\n", ch, readADC(ch));
    }

    return 0;
}