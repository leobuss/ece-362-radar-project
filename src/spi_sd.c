#include "header.h"  
#include "sd_header.h"

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

    // need to continue later
    

    
}

uint8_t sd_transfer(uint8_t data_in)
{
    uint8_t output = 0x00;
    spi_write_read_blocking(spi1, &data_in, &output, 1);
    return output;
}

void sd_select()
{
    gpio_put(SD_CSn, 0);
    sd_transfer(0xFF);
}

void sd_deselect()
{
    gpio_put(SD_CSn, 1);
    sd_transfer(0xFF);
}

uint8_t send_sd_command(uint8_t command, uint32_t argument, uint8_t crc, uint8_t extra_data_len, uint8_t* extra_data)
// include extra room for return data that is more than 1 byte
{
    sd_select();

    // format of a data packet - always send 6 bytes at a time
    uint8_t data[6] = {command, (uint8_t)(argument >> 24), (uint8_t)(argument >> 16), (uint8_t)(argument >> 8), (uint8_t)(argument), crc};
    spi_write_blocking(spi1, data, 6);

    // need to continue

}