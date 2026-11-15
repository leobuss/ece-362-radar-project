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


#include "hardware/sync.h"

// timer flag for the radar test
static volatile bool radar_tick = false;

// timer interrupt: signal that the radar needs another update
static bool radar_test_irq(struct repeating_timer *t)
{
    radar_tick = true;
    return true; // keep timer running
}

// integrated radar test: simulated servo + detections + lcd
void radar_test(void)
{
    // initialize lcd and clear previous detections
    lcd_init();
    radar_reset();

    // simulated servo position and movement direction
    int angle = -90;
    int direction = 1;
    int step = 5;

    // set up timer to trigger radar updates every 100ms
    struct repeating_timer timer;
    radar_tick = false;

    if (!add_repeating_timer_ms(100, radar_test_irq, NULL, &timer))
    {
        printf("radar timer failed\n");
        return;
    }

    while (true)
    {
        // safely check and clear timer flag
        uint32_t interrupts = save_and_disable_interrupts();
        bool update_due = radar_tick;
        radar_tick = false;
        restore_interrupts(interrupts);

        if (update_due)
        {
            // simulate objects at different angles
            int distance = TOF_NO_TARGET;

            if (angle == -45)
                distance = 1800;
            else if (angle == 0)
                distance = 3000;
            else if (angle == 45 && direction == 1)
                distance = 4500;

            // update sweep line and detection dots on lcd
            radar_update(angle, distance);

            // move simulated servo to next position
            angle += direction * step;

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

        tight_loop_contents();
    }
}
