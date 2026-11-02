#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "sd_header.h"
#include "i2c.h"    

// tft lcd functions
void lcd_init(void);
void lcd_fill_screen(uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);
void lcd_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void lcd_draw_semicircle(int xc, int yc, int radius, uint16_t color);

// pwm functions
void pwm_start(void);
void pwm_seek(int pos);


// sd card functions
void init_spi();
bool init_sd_spi();
uint8_t sd_transfer(uint8_t data_in);
void sd_select();
void sd_deselect();
uint8_t send_sd_command(uint8_t command, uint32_t argument, uint8_t crc, uint8_t* extra_data, uint8_t extra_data_len);


//testing functions

void pwm_test(void);
void display_test(void);
