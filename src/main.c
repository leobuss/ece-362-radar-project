#include "header.h"

// #define DISPLAY_TEST // uncomment to test display func

// #define DISPLAY_TEST
#define PWM_TEST // uncomment to test display func
// #define SD_TEST

// main
int main(void)
{
    stdio_init_all();
    #ifdef DISPLAY_TEST
        display_test();
    #endif
    #ifdef PWM_TEST
        printf("calling pwm test");
        pwm_test();
    #endif
    #ifdef SD_TEST
        init_spi();
        if(init_sd_spi) printf("SD card initialized");
        else printf("SD card failed to initialize");
    #endif
    while (true){
        tight_loop_contents();
    }
    return 0;
}