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


#include "hardware/sync.h"

// onboard buttons
#define MODE_BUTTON 21
#define HISTORY_BUTTON 26

#define HISTORY_COUNT 3

// timer flag for the radar test
static volatile bool radar_tick = false;

// fictional historical scans (one distance per degree)
static const int mock_history[HISTORY_COUNT][181] =
{
    // scan 1: newest scan
    {
        [45] = 1800,
        [90] = 3000,
        [135] = 4500
    },

    // scan 2: older scan
    {
        [30] = 2200,
        [90] = 3500,
        [150] = 2800
    },

    // scan 3: oldest scan
    {
        [60] = 1400,
        [110] = 4200,
        [160] = 5000
    }
};

// fictional age of each scan in minutes
static const int mock_minutes[HISTORY_COUNT] =
{
    1, 5, 10
};

// timer interrupt: signal that the radar needs another update
static bool radar_test_irq(struct repeating_timer *t)
{
    radar_tick = true;
    return true;
}

// initialize onboard buttons
static void radar_init_buttons(void)
{
    gpio_init(MODE_BUTTON);
    gpio_set_dir(MODE_BUTTON, GPIO_IN);

    gpio_init(HISTORY_BUTTON);
    gpio_set_dir(HISTORY_BUTTON, GPIO_IN);
}

// integrated radar test: live scan and mock scan history
void radar_test(void)
{
    // initialize lcd and buttons
    lcd_init();
    radar_init_buttons();

    // start in live scan mode
    bool history_mode = false;
    int history_index = 0;

    radar_reset();
    radar_draw_mode(false, 0);

    // simulated servo position and movement direction
    int angle = -90;
    int direction = 1;
    int step = 5;

    // remember previous button states
    bool prev_mode = gpio_get(MODE_BUTTON);
    bool prev_history = gpio_get(HISTORY_BUTTON);

    // set up timer for radar updates every 100ms
    struct repeating_timer timer;
    radar_tick = false;

    if (!add_repeating_timer_ms(100, radar_test_irq,
                                NULL, &timer))
    {
        printf("radar timer failed\n");
        return;
    }

    while (true)
    {
        // read current button states
        bool mode_button = gpio_get(MODE_BUTTON);
        bool history_button = gpio_get(HISTORY_BUTTON);

        // toggle between live scan and scan history
        if (mode_button && !prev_mode)
        {
            history_mode = !history_mode;

            if (history_mode)
            {
                // open newest historical scan
                history_index = 0;

                radar_show_history(
                    mock_history[history_index],
                    mock_minutes[history_index],
                    history_index + 1,
                    HISTORY_COUNT
                );
            }
            else
            {
                // clear history and resume live scan
                radar_reset();
                radar_draw_mode(false, 0);

                angle = -90;
                direction = 1;
            }
        }

        // cycle through historical scans
        if (history_mode &&
            history_button && !prev_history)
        {
            history_index++;

            // wrap around to newest scan
            if (history_index >= HISTORY_COUNT)
                history_index = 0;

            radar_show_history(
                mock_history[history_index],
                mock_minutes[history_index],
                history_index + 1,
                HISTORY_COUNT
            );
        }

        // save button states for next iteration
        prev_mode = mode_button;
        prev_history = history_button;

        // safely check and clear timer flag
        uint32_t interrupts = save_and_disable_interrupts();
        bool update_due = radar_tick;
        radar_tick = false;
        restore_interrupts(interrupts);

        // only update the live radar in live scan mode
        if (update_due && !history_mode)
        {
            // simulate objects at different angles
            int distance = TOF_NO_TARGET;

            if (angle == -45)
                distance = 1800;
            else if (angle == 0)
                distance = 3000;
            else if (angle == 45 && direction == 1)
                distance = 4500;

            // update sweep, detections, and text fields
            radar_update(angle, distance);
            radar_draw_fields(angle, distance);

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
