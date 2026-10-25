#include "header.h"

// tft lcd spi0 (fill out with gpio)
#define TFT_SCK
#define TFT_MOSI
#define TFT_CS
#define TFT_DC
#define TFT_RST

void display_init_spi(void) 
{
    spi_init(spi0, 1000000); // 1MHz

    //config sck and mosi
    gpio_set_function(TFT_SCK, GPIO_FUNC_SPI);
    gpio_set_function(TFT_MOSI, GPIO_FUNC_SPI);

    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // 8b mode 0 msb first

    // configure as gpio out
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

void lcd_write_command(uint8_t command)
{
    gpio_put(TFT_CS, 0); // command mode
    gpio_put(TFT_DC, 0); // select disp
    spi_wite_blocking(spi0, &command, 1) // send command
    gpio_put(TFT_CS, 1); // deselect disp
}

void lcd_write_data(uint8_t data)
{
    gpio_put(TFT_DC, 1); // data mode
    gpio_put(TFT_CS, 0); // select disp
    spi_write_blocking(spi0, &data, 1); // send
    gpio_put(TFT_CS, 1); // deselect disp
}

void lcd_reset(void)
{
    gpio_put(TFT_RST, 0); // hardware rst
    sleep_ms(20); // at least 10us valid pulse
    gpio_put(TFT_RST, 1); // release
    sleep_ms(120); // at least 120ms before sleep out
}