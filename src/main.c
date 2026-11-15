#include "header.h"


// Timing parameters
#define SERVO_SETTLE_MS 20
#define TOF_CHECK_DELAY_MS 50
#define TOF_RECHECK_MS 5
#define TOF_TIMEOUT_MS 500

#define DETECTION_THRESHOLD_MM 3000
#define SERVO_STEP 5 // in degrees

// timer interrupt flags
static volatile bool servo_settled = false;
static volatile bool tof_check_due = false;
static volatile bool timer_error = false;

// Current servo state
static volatile int angle = -90;
static int direction = 1;

// time the current measurement started
static uint64_t tof_start_us = 0;

// irq functions
int64_t move_servo_irq(alarm_id_t id, void *data);
int64_t servo_settled_irq(alarm_id_t id, void *data);
int64_t tof_check_irq(alarm_id_t id, void *data);

// test functions
// #define DISPLAY_TEST
// #define PWM_TEST
// #define SD_TEST
// #define I2C_TEST 
#define RADAR_TEST
// #define MAIN
// main

void schedule_timer(uint32_t delay_ms, alarm_callback_t callback)
{
    add_alarm_in_ms(delay_ms, callback, NULL, false);
}

// Move servo, then set timer for settling
int64_t move_servo_irq(alarm_id_t id, void *data)
{
    pwm_seek(angle);
    schedule_timer(SERVO_SETTLE_MS, servo_settled_irq);
    return 0;
}

// servo has settled
int64_t servo_settled_irq(alarm_id_t id, void *data)
{
    servo_settled = true;
    return 0;
}

// need a tof check
int64_t tof_check_irq(alarm_id_t id, void *data)
{
    tof_check_due = true;
    return 0;
}

// process reading and schedule next position
void process_measurement(int distance)
{
    int measured_angle = angle;

    bool detected = distance > 0 && distance <= DETECTION_THRESHOLD_MM;

    // anything outside of threshold ignore
    int display_distance = distance;
    if (distance > DETECTION_THRESHOLD_MM)
    {
        display_distance = TOF_NO_TARGET;
    }

    if (detected)
    {
        // sd card logging function once the sd card actually works
        printf("detected object at %d degrees and %dm", angle, distance);
    }

    // go to next angle
    int next_angle = angle + direction * SERVO_STEP;

    if (next_angle > 90 || next_angle < -90)
    {
        direction = -direction;
        next_angle = angle + direction * SERVO_STEP;
    }

    angle = next_angle;

    // state next servo movement
    schedule_timer(1, move_servo_irq);

    // update display
    radar_update(measured_angle, display_distance);
}
int main(void)
{
    stdio_init_all();

    
    sleep_ms(1000);

    #ifdef MAIN

        if (!tof_init())
        {
            printf("ToF initialization failed\n");
            return 1;
        }
        lcd_init();
        pwm_start();
        radar_draw_background();
        // start moving servo
        pwm_seek(-90);
        schedule_timer(300, servo_settled_irq);

        while (true)
        {
            if (servo_settled) // servo is at final position
            {
                servo_settled = false;
                if (tof_begin_measurement() != 0)
                {
                    process_measurement(TOF_ERROR);
                }
                else
                {
                    tof_start_us = time_us_64();
                    // measurement after delay
                    schedule_timer(TOF_CHECK_DELAY_MS, tof_check_irq);
                }
            }
            if (tof_check_due)
            {
                tof_check_due = false;

                int distance = TOF_NO_TARGET;
                int status = tof_read_if_ready(&distance);

                if (status == 1) // measurement done
                {
                    process_measurement(distance);
                }
                else if (status == 0) // measurement still in progress
                {
                    if (time_us_64() - tof_start_us >= TOF_TIMEOUT_MS * 1000ULL)
                    {
                        process_measurement(TOF_ERROR);
                    }
                    else
                    {
                        schedule_timer(TOF_RECHECK_MS, tof_check_irq);
                    }
                }
                else
                {
                    process_measurement(TOF_ERROR);
                }
            }

        }
    #endif






    #ifdef DISPLAY_TEST
        display_test();
    #endif
    #ifdef PWM_TEST
        printf("calling pwm test");
        pwm_test();
    #endif
    #ifdef SD_TEST
        printf("initializing spi\n");
        fflush(stdout);
        sd_test();
    #endif
    #ifdef I2C_TEST
        i2c_test();
    #endif
    #ifdef RADAR_TEST
        printf("Starting integrated radar test \n");
        radar_test();
    #endif
    while (true){
        tight_loop_contents();
    }
    
    return 0;
}