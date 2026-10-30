#include "header.h"

// tft lcd spi0 (fill out with gpio, change if needed)
#define TFT_SCK 2
#define TFT_MOSI 3
#define TFT_CS 5
#define TFT_DC 6
#define TFT_RST 7

// colors
#define BLACK 0x0000
#define GREEN 0x07E0
#define RED 0xF800
#define BLUE 0x001F

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
    gpio_init(TFT_RST);
    gpio_set_dir(TFT_RST, GPIO_OUT);
    gpio_put(TFT_RST, 1);
}

// send an 8-bit command to ILI9341 over spi0
void lcd_write_command(uint8_t command)
{
    gpio_put(TFT_DC, 0); // command mode
    gpio_put(TFT_CS, 0); // select disp
    spi_write_blocking(spi0, &command, 1); // send command
    gpio_put(TFT_CS, 1); // deselect disp
}

// send an 8-bit data value or command parameter to ILI9341
void lcd_write_data(uint8_t data)
{
    gpio_put(TFT_DC, 1); // data mode
    gpio_put(TFT_CS, 0); // select disp
    spi_write_blocking(spi0, &data, 1); // send
    gpio_put(TFT_CS, 1); // deselect disp
}

// send multiple data bytes in one spi transaction (bulk writing ex fill screen)
void lcd_write_buffer(const uint8_t *buffer, size_t length)
{
    gpio_put(TFT_DC, 1); // data mode
    gpio_put(TFT_CS, 0); // select disp
    spi_write_blocking(spi0, buffer, length); // send
    gpio_put(TFT_CS, 1); // deselect disp
}

// perform a hardware reset of lcd controller before sending initilization
void lcd_reset(void)
{
    gpio_put(TFT_RST, 0); // hardware rst
    sleep_ms(20); // at least 10us valid pulse
    gpio_put(TFT_RST, 1); // release
    sleep_ms(120); // at least 120ms before sleep out
}

// initiliaze tft display contorller: reset hardware, exit sleep, configure rgb565 color
// landscape oritentation with bgr ordering and enable display out
void lcd_init(void)
{
    display_init_spi();
    lcd_reset();
    lcd_write_command(0x11); // exit sleep
    sleep_ms(120); // sleep out transition

    lcd_write_command(0x3A); // pixel format
    lcd_write_data(0x55); // 16b rgb565
    lcd_write_command(0x36); // mem access
    lcd_write_data(0x28); // landscape + bgr order
    lcd_write_command(0x29); // enable lcd out
}

// select rectangular area of display memory to write to
// 0x2A sets X coords, 0x2B sets y coords, 0x2C starts pixel memory writing
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    lcd_write_command(0x2A); // column range
    lcd_write_data(x0 >> 8); // start x hight byte
    lcd_write_data(x0 & 0xFF); // start x low byte
    lcd_write_data(x1 >> 8); // end x high byte
    lcd_write_data(x1 & 0xFF); // end x low byte

    lcd_write_command(0x2B); // row range
    lcd_write_data(y0 >> 8); // start y high byte
    lcd_write_data(y0 & 0xFF); // start y low byte
    lcd_write_data(y1 >> 8); // end y high byte
    lcd_write_data(y1 & 0xFF); // end y low byte

    lcd_write_command (0x2C); // start writing pixel memory
}

// draw a single pixel at (x, y) using 16-bit rgb565 color
// select one pixel with set_window and send its two color bytes
void lcd_draw_pixel(int x, int y, uint16_t color)
{
    // check if within acceptable values
    if (x < 0 || x >= 320 || y < 0 || y >= 240)
    {
        return;
    }
    lcd_set_window(x, y, x, y);
    lcd_write_data(color >> 8); // color high byte
    lcd_write_data(color & 0xFF);
}

// fill entire 320x240 display with one rgb565 color
void lcd_fill_screen(uint16_t color)
{
    uint8_t buffer[128]; // 64 pixels, 2 bytes each
    for (int i = 0; i < 64; i++) // fill buffer with color
    {
        buffer[2*i] = color >> 8; // high byte
        buffer[2*i+1] = color & 0xFF; // low byte
    }
    lcd_set_window(0, 0, 319, 239); // entire 320x240
    for (int i = 0; i < 1200; i++) // 1200 batches for 76800 pixels
    {
        lcd_write_buffer(buffer, 128);
    }
}

// bresenham's line algo: draws a line between two points using integer error calculations to determine which pixels to plot
void lcd_draw_line(int x0, int y0, int x1, int y1, uint16_t color)
{
    int dx = abs(x1 - x0); // horiz dist between endpoints
    int dy = abs(y1 - y0); // vert dist between endpoints
    int sx = (x0 < x1) ? 1 : -1; // direction along x
    int sy = (y0 < y1) ? 1: -1; // direction along y
    int err = dx - dy; // track plotted pixel alignment with ideal line
    while (1)
    {
        lcd_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
        {
            break;
        }
        int e2 = 2 * err; // decide to advance x, y, or both
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// draw an upper semicircle by plotting pixels from 0 to 180 degrees
void lcd_draw_semicircle(int xc, int yc, int radius, uint16_t color)
{
    for (int angle = 0; angle <= 180; angle++)
    {
        float rad = angle * 3.14159265f / 180.0f;

        int x = xc + radius * cosf(rad);
        int y = yc - radius * sinf(rad);

        lcd_draw_pixel(x, y, color);
    }
}