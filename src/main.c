#include "header.h"

// #define DISPLAY_TEST // uncomment to test display func
// #define PWM_TEST // uncomment to test display func
#define SD_TEST
// #define I2C_TEST 
// #define RADAR_TeST
// main
int main(void)
{
    stdio_init_all();

    
    sleep_ms(1000);
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