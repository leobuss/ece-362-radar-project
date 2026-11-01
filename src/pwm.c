#include "header.h"

#define PWM_PIN 27

uint slice_num;
#define div 60
#define wrap 49999

void pwm_start(void){
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    pwm_set_wrap(slice_num, wrap); //set frequency
    pwm_set_clkdiv(slice_num, div);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 3750); //set duty
    pwm_set_enabled(slice_num, true);
}

void pwm_seek(int pos){
    //Range is +/- 90 degrees
    if(pos > 90){ //clip degree input into bounds
        pos = 90;
    }
    if(pos < -90){
        pos = -90;
    }
    pwm_set_chan_level(slice_num, PWM_CHAN_B, (2500 + (pos + 90) / 180.0 * 2500));
}