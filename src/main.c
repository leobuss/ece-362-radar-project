#include "header.h"

// #define DISPLAY_TEST // uncomment to test display func

// #define DISPLAY_TEST
// #define PWM_TEST // uncomment to test display func
#define SD_TEST
//#define I2C_TEST 
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
        init_spi();
        printf("running SD card test\n");
        if(init_sd_spi()) {printf("SD card initialized\n"); fflush(stdout);}
        else {printf("\nSD card failed to initialize\n"); fflush(stdout);}
    #endif
    #ifdef I2C_TEST
        i2c_test();
    #endif
    while (true){
        tight_loop_contents();
    }
    
    return 0;
}