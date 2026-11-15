#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "sd/sd_header.h"
#include "i2c/i2c.h"    
#include "sd/ff.h"
#include "sd/diskio.h"
#include "pico/time.h"

// tft lcd functions
void lcd_init(void);
void lcd_fill_screen(uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);
void lcd_draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void lcd_draw_semicircle(int xc, int yc, int radius, uint16_t color);
void lcd_draw_dot(int xc, int yc, int radius, uint16_t color);
void lcd_draw_char(int x, int y, char c, uint16_t color, uint16_t bg, int size);
void lcd_draw_string(int x, int y, const char *str, uint16_t color, uint16_t bg, int size);
void radar_draw_background(void);
void radar_reset(void);
void radar_update(int servo_angle, int distance_mm);
void radar_draw_fields(int servo_angle, int distance_mm);
void radar_draw_mode(bool history, int minutes_ago);
void radar_show_history(const int distances[181], int minutes_ago, int scan_number, int total_scans);


// pwm functions
void pwm_start(void);
void pwm_seek(int pos);


// sd card functions
void sd_test();
uint8_t sdcard_write(uint8_t b);


// testing functions
void pwm_test(void);
void display_test(void);
void radar_test(void);
