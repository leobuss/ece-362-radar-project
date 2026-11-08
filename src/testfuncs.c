#include "header.h"

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

void pwm_test(void){
    printf("running pwm_test");
    pwm_start();
    bool inc = true;
    int degree = -90;
    for(;;){
        pwm_seek(degree);
        if((!inc && degree <= -90) || (inc && degree >= 90)){
            inc = !inc;
        }
        degree = degree - 1 + (inc * 2);
        sleep_ms(10); 
    }
}

// integrated radar test: servo + ToF sensor + LCD
void radar_test(void)
{
    // initialize LCD and servo
    lcd_init();
    pwm_start();

    // initialize distance sensor
    if (!tof_init())
    {
        printf("ToF initialization failed\n");
        return;
    }

    // draw static radar grid once
    radar_draw_background();

    // servo position and movement direction
    int angle = -90;
    int direction = 1;

    while (true)
    {
        // move servo to current angle
        pwm_seek(angle);

        // allow servo time to move
        sleep_ms(100);

        // read distance at current angle
        int distance = tof_read_distance_mm();

        // update LCD sweep and plot detected objects
        radar_update(angle, distance);

        // move 5 degrees per measurement
        angle += direction * 5;

        // reverse direction at sweep boundaries
        if (angle >= 90)
        {
            angle = 90;
            direction = -1;
        }
        else if (angle <= -90)
        {
            angle = -90;
            direction = 1;
        }
    }
}