#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// tft lcd spi0 (fill out with gpio)
#define TFT_SCK
#define TFT_MOSI
#define TFT_CS
#define TFT_DC
#define TFT_RST

void display_init_spi(void)
{
    // initiliaze spi0 at 1 megahz
    spi_init(spi0, 1000000);

    //config sck and mosi
    gpio_set_function(TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_MOSI, GPIO_FUNC_SPI);

    // set spi format to 8b mode 0 msb
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // configure cs, dc, reset as gpio out
    gpio_init(TFT_CS);
    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_put(TFT_CS, 1);
    gpio_init(TFT_DC);
    gpio_set_dir(TFT_DC, GPIO_OUT);
    gpio_put(TFT_DC, 1);
    gpio_init(TFT_CS);
    gpio_set_dir(TFT_CS, GPIO_OUT);
    gpio_put(TFT_RST, 1);
}