#include "header.h"  

#define SD_MOSI 31
#define SD_MISO 28 // add in 10k ohm pull up
#define SD_SCK 30
#define SD_CSn 29

struct reading
{
    float angle;
    float distance;
    uint32_t time;
};


void init_spi_sdcard() {
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI); gpio_set_function(SD_MISO, GPIO_FUNC_SPI); gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_set_dir(SD_CSn, GPIO_OUT);
    gpio_put(SD_CSn, 1);  
    gpio_set_function(SD_CSn, GPIO_FUNC_SIO); 
    spi_init(spi1, 100000); spi_set_format(spi1, 8, 0, 0, SPI_MSB_FIRST);
}

void disable_sdcard() {
    gpio_put(SD_CSn, 1);
    sdcard_write(0xff);
    gpio_put(SD_MOSI, 1);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SIO);
    gpio_set_dir(SD_MOSI, GPIO_OUT);
    
    
}

void enable_sdcard() {
    gpio_put(SD_CSn, 0);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI);
    
}

void sdcard_io_high_speed() {
    // spi_set_baudrate(spi1, 12000000);
}

void init_sdcard_io() {
    init_spi_sdcard();
    disable_sdcard();
}


void sd_test()
{
    init_uart();
    init_uart_irq();
    init_sdcard_io();
    command_shell();

    for(;;);
}

