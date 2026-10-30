#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// tft lcd functions
void lcd_init(void);
void lcd_fill_screen(uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);
void lcd_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void lcd_draw_semicircle(int xc, int yc, int radius, uint16_t color);