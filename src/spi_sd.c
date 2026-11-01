#include "header.h"  

#define SD_MOSI 31
#define SD_MISO 28
#define SD_SCK 30
#define SD_CSn 29

struct reading
{
    float angle;
    float distance;
    uint32_t time;
};

void init_spi()
{
    gpio_init(SD_CSn); gpio_init(SD_MOSI); gpio_init(SD_MISO); gpio_init(SD_SCK);
    gpio_set_function(SD_MOSI, GPIO_FUNC_SPI); gpio_set_function(SD_MISO, GPIO_FUNC_SPI); gpio_set_function(SD_SCK, GPIO_FUNC_SPI);
    gpio_pull_up(SD_MISO);
    gpio_pull_up(SD_MOSI);
    gpio_put(SD_CSn, 1); gpio_set_dir(SD_CSn, GPIO_OUT); 
    gpio_pull_up(SD_CSn);
    
  

    spi_init(spi1, 400000);
    spi_set_format(spi1, 8, 0, 0, SPI_MSB_FIRST);

}

bool init_sd_spi()
{
    
    gpio_put(SD_CSn, 1);
    sleep_ms(10);

    for (int i = 0; i < 10; i++)
    {
        sd_transfer(0xFF); // send 1s to initalize SD Card
    }

    // enter spi idle mode
    uint8_t r = 0xFF; // 0x95 is required CRC byte
    for (int i = 0; i < 10; i++)
    {
        r = send_sd_command(CMD0, 0, 0x95, NULL, 0);
        sd_deselect();
        if (r == 0x01) {
            break;
        }
        sleep_ms(10);
    }
    if (r != 0x01) {printf("r != 0x01; r = 0x%02X", r); return false;}

    // check card version and voltage to ensure proper card
    uint8_t data[4]; 
    r = send_sd_command(CMD8, 0x1AA, 0x87, data, 4); // 0x87 is required CRC byte
    sd_deselect();
    if (r != 0x01 || data[2] != 0x01 || data[3] != 0xAA) {printf("r != 0x01 || data[2] != 0x01 || data[3] != 0xAA"); return false;}

    // wait for card to initialize
    bool ready = false;

    for (int i = 0; i < 200; i++) // give it 2 seconds to get ready otherwise fail
    {
        r = send_sd_command(CMD55, 0, 0x01, NULL, 0);
        sd_deselect();
        if (r != 0x01 && r != 0x00) {printf("r != 0x01 && r != 0x00\n"); return false;}

        r = send_sd_command(ACMD41, 0x40000000, 0x01, NULL, 0);
        sd_deselect();

        if (r == 0x00) {
            ready = true;
            break;
        }
        if (r != 0x01){printf("r != 0x01\n r = 0x%02X", r); return false;} 

        sleep_ms(10);
    }

    if (!ready) {printf("not ready\n"); return false;}

    uint8_t ocr[4];
    r = send_sd_command(CMD58, 0, 0x01, ocr, 4);
    sd_deselect();

    if (r != 0x00 || (ocr[0] & 0xC0) != 0xC0){printf("r != 0x00 || (ocr[0] & 0xC0) != 0xC0\n"); return false;}

    return true;


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

uint8_t send_sd_command(uint8_t command, uint32_t argument, uint8_t crc, uint8_t* extra_data, uint8_t extra_data_len)
// include extra room for return data that is more than 1 byte
{
    sd_select();

    // format of a data packet - always send 6 bytes at a time
    uint8_t data[6] = {0x40 | command, (uint8_t)(argument >> 24), (uint8_t)(argument >> 16), (uint8_t)(argument >> 8), (uint8_t)(argument), crc};
    spi_write_blocking(spi1, data, 6);
    uint8_t response = 0xFF;
    for (int i = 0; i < 10; i++) {
        response = sd_transfer(0xFF);
        // valid response has MSB = 0
        if (!(response & 0x80)) {
            break;
        }
    }

    if (response != 0xFF && extra_data != NULL) {
        for (size_t i = 0; i < extra_data_len; i++) {
            extra_data[i] = sd_transfer(0xFF);
        }
    }

    return response;

}