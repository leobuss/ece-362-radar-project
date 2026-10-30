#include "header.h"

// #define DISPLAY_TEST // uncomment to test display func
#ifdef DISPLAY_TEST
// test tft display: draw a static radar grid and yellow sweep line
void display_test(void)
{
    lcd_init();
    lcd_fill_screen(0x0000); // black background

    int xc = 160;
    int yc = 205;
    int radius = 140;

    // draw semicircular range rings
    lcd_draw_semicircle(xc, yc, 35, 0x03E0);
    lcd_draw_semicircle(xc, yc, 70, 0x03E0);
    lcd_draw_semicircle(xc, yc, 105, 0x03E0);
    lcd_draw_semicircle(xc, yc, 140, 0x07E0);

    // draw faint green radial lines every 30 degrees
    for (int angle = 30; angle <= 150; angle += 30)
    {
        float rad = angle * 3.14159265f / 180.0f;

        int x = xc + radius * cosf(rad);
        int y = yc - radius * sinf(rad);

        lcd_draw_line(xc, yc, x, y, 0x0280);
    }

    // draw horizontal radar baseline
    lcd_draw_line(20, 205, 300, 205, 0x07E0);

    // draw yellow scanning line at 55 degrees
    float rad = 55 * 3.14159265f / 180.0f;

    int x = xc + radius * cosf(rad);
    int y = yc - radius * sinf(rad);

    lcd_draw_line(xc, yc, x, y, 0xFFE0);
}

#endif

// main
int main(void)
{
    stdio_init_all();
    #ifdef DISPLAY_TEST
        display_test();
    #endif
    while (true){
        tight_loop_contents();
    }
    return 0;
}