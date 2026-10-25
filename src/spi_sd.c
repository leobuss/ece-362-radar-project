#include "header.h"  

#define SD_MOSI 1
#define SD_MISO 2
#define SD_SCK 3
#define SD_CSn 4

struct reading
{
    float angle;
    float distance;
    uint32_t time;
};

void init_sdspi()
{
    // init gpios for spi
    gpio_init(SD_MOSI); gpio_init(SD_MISO); gpio_init(SD_SCK); gpio_init(SD_CSn);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI); gpio_set_function(SD_MOSI, GPIO_FUNC_SPI); gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_set_dir(SD_CSn, GPIO_OUT); gpio_put(SD_CSn, 1);

    // init the spi peripheral
    spi_init(spi1, 100000);
    spi_set_format(spi1, 8, 0, 0, SPI_MSB_FIRST);

    for (int i = 0; i < 10; i++)
    {
        sd_transfer(0xFF); // send 1s to initalize SD Card
    }


    

    
}

uint8_t sd_transfer(uint8_t data_in)
{
    uint8_t output = 0x00;
    spi_write_read_blocking(spi1, &data_in, &output, 1);
    return output;
}