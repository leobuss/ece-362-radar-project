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

// compact 2-bit memory of the static radar background
#define LCD_WIDTH 320
#define LCD_HEIGHT 240

static uint8_t radar_bg[LCD_WIDTH * LCD_HEIGHT / 4];
static bool radar_recording = false;

// save background colors using 2 bits per pixel
static void radar_save_bg_pixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT)
        return;

    int pixel = y * LCD_WIDTH + x;
    int byte = pixel / 4;
    int shift = (pixel % 4) * 2;

    uint8_t code = 0;

    if (color == 0x0280) code = 1;
    else if (color == 0x03E0) code = 2;
    else if (color == 0x07E0) code = 3;

    radar_bg[byte] =
        (radar_bg[byte] & ~(3u << shift)) | (code << shift);
}

// retrieve the original background color at any pixel
static uint16_t radar_get_bg_pixel(int x, int y)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT)
        return BLACK;

    int pixel = y * LCD_WIDTH + x;
    int shift = (pixel % 4) * 2;

    uint8_t code = (radar_bg[pixel / 4] >> shift) & 3;

    switch (code)
    {
        case 1: return 0x0280;
        case 2: return 0x03E0;
        case 3: return 0x07E0;
        default: return BLACK;
    }
}

void display_init_spi(void) 
{
    spi_init(spi0, 10000000); // 10MHz

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
    if (radar_recording)
    {
        radar_save_bg_pixel(x, y, color);
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

// draw filled circular marker for detected obkects
void lcd_draw_dot(int xc, int yc, int radius, uint16_t color)
{
    for (int y = -radius; y <= radius; y++)
    {
        for (int x = -radius; x <= radius; x++)
        {
            if (x*x + y*y <= radius*radius)
            {
                lcd_draw_pixel(xc + x, yc + y, color);
            }
        }
    }
}

// plot deteteced object based on servo angle and distance
void radar_draw_object(int servo_angle, int distance_mm)
{
    // ignore invalid or out of range readings
    if (distance_mm <= 0 || distance_mm > 6000)
    {
        return;
    }

    // radar display dimensions
    int xc = 160;
    int yc = 205;
    int radius = 140;

    // convert servo angle (-90 to +90) into radar angle (0 to 180)
    int radar_angle = servo_angle + 90;

    // convert angle from degrees to radians
    float rad = radar_angle * 3.14159265f / 180.0f;

    // scale distance (0-6000 mm) to display radius (0-140 pixels)
    float r = ((float)distance_mm / 6000.0f) * radius;

    // calculate object position on LCD
    int x = xc + r * cosf(rad);
    int y = yc - r * sinf(rad);

    // draw red detection marker
    lcd_draw_dot(x, y, 3, 0xF800);
}

// initialize the static radar grid and save its background colors
void radar_draw_background(void)
{
    memset(radar_bg, 0, sizeof(radar_bg));

    lcd_fill_screen(BLACK);

    const int xc = 160;
    const int yc = 205;
    const int radius = 140;

    // start recording the static grid pixels
    radar_recording = true;

    // range rings
    lcd_draw_semicircle(xc, yc, 35, 0x03E0);
    lcd_draw_semicircle(xc, yc, 70, 0x03E0);
    lcd_draw_semicircle(xc, yc, 105, 0x03E0);
    lcd_draw_semicircle(xc, yc, 140, 0x07E0);

    // radial lines
    for (int angle = 30; angle <= 150; angle += 30)
    {
        float rad = angle * 3.14159265f / 180.0f;

        int x = xc + radius * cosf(rad);
        int y = yc - radius * sinf(rad);

        lcd_draw_line(xc, yc, x, y, 0x0280);
    }

    // horizontal baseline
    lcd_draw_line(20, 205, 300, 205, 0x07E0);

    // stop recording; subsequent updates won't change the grid map
    radar_recording = false;
}

// persistent detection memory
static int radar_distances[37] = {0};
static int radar_dot_x[37];
static int radar_dot_y[37];
static int radar_prev_angle = -1;

// returns the correct pixel color without redrawing the whole grid
static uint16_t radar_pixel_color(int x, int y, bool sweep)
{
    // detected objects take priority over the yellow sweep
    for (int i = 0; i < 37; i++)
    {
        if (radar_distances[i] <= 0)
            continue;

        int dx = x - radar_dot_x[i];
        int dy = y - radar_dot_y[i];

        if (dx*dx + dy*dy <= 9)
            return RED;
    }

    if (sweep)
        return 0xFFE0;

    return radar_get_bg_pixel(x, y);
}

// restore or draw a sweep line using the current detection map
static void radar_paint_sweep(int angle, bool yellow)
{
    const int xc = 160;
    const int yc = 205;
    const int radius = 140;

    float rad = angle * 3.14159265f / 180.0f;

    int x1 = xc + radius * cosf(rad);
    int y1 = yc - radius * sinf(rad);

    // Bresenham line drawing
    int x = xc;
    int y = yc;

    int dx = abs(x1 - x);
    int dy = abs(y1 - y);
    int sx = (x < x1) ? 1 : -1;
    int sy = (y < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        uint16_t color = radar_pixel_color(x, y, yellow);
        lcd_draw_pixel(x, y, color);

        if (x == x1 && y == y1)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

// repaint only the pixels around a changed detection
static void radar_paint_dot_area(int xc, int yc)
{
    for (int y = yc - 3; y <= yc + 3; y++)
    {
        for (int x = xc - 3; x <= xc + 3; x++)
        {
            if ((x-xc)*(x-xc) + (y-yc)*(y-yc) <= 9)
            {
                lcd_draw_pixel(
                    x, y, radar_pixel_color(x, y, false)
                );
            }
        }
    }
}

// initialize the current detection map and update the radar
void radar_update(int servo_angle, int distance_mm)
{
    const int xc = 160;
    const int yc = 205;
    const int radius = 140;

    int radar_angle = servo_angle + 90;

    if (radar_angle < 0) radar_angle = 0;
    if (radar_angle > 180) radar_angle = 180;

    int index = (radar_angle + 2) / 5;

    // remember previous detection position at this angle
    int old_distance = radar_distances[index];
    int old_x = radar_dot_x[index];
    int old_y = radar_dot_y[index];

    // update only the angle currently being scanned
    if (distance_mm > 0 && distance_mm <= 6000)
    {
        radar_distances[index] = distance_mm;

        float rad = (index * 5) * 3.14159265f / 180.0f;
        float r = ((float)distance_mm / 6000.0f) * radius;

        radar_dot_x[index] = xc + r * cosf(rad);
        radar_dot_y[index] = yc - r * sinf(rad);
    }
    else if (distance_mm == TOF_NO_TARGET)
    {
        radar_distances[index] = 0;
    }
    // TOF_ERROR preserves the previous detection

    // restore only the previous yellow sweep
    if (radar_prev_angle >= 0)
    {
        radar_paint_sweep(radar_prev_angle, false);
    }

    // restore the previous detection location if it changed
    if (old_distance > 0 &&
        old_distance != radar_distances[index])
    {
        radar_paint_dot_area(old_x, old_y);
    }

    // draw the detection at its updated location
    if (radar_distances[index] > 0)
    {
        radar_paint_dot_area(
            radar_dot_x[index], radar_dot_y[index]
        );
    }

    // draw the new sweep without covering red detection dots
    radar_paint_sweep(radar_angle, true);

    // save current angle for the next update
    radar_prev_angle = radar_angle;
}