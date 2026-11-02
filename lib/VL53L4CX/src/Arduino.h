// Arduino.h - stand-in so ST's VL53L4CX driver (written for Arduino) builds with the Pico SDK.
// The driver only needs timing (millis/delay) and the xshut pin (pinMode/digitalWrite),


#ifndef VL53L4CX_ARDUINO_SHIM_H
#define VL53L4CX_ARDUINO_SHIM_H

#include <stdint.h>
#include "pico/stdlib.h"

#define INPUT  0
#define OUTPUT 1
#define LOW    0
#define HIGH   1

static inline uint32_t millis(void)
{
    return to_ms_since_boot(get_absolute_time());
}

static inline void delay(uint32_t ms)
{
    sleep_ms(ms);
}

static inline void delayMicroseconds(uint32_t us)
{
    sleep_us(us);
}

static inline void pinMode(int pin, int mode)
{
    gpio_init(pin);
    gpio_set_dir(pin, mode == OUTPUT);
}

static inline void digitalWrite(int pin, int value)
{
    gpio_put(pin, value);
}

#endif
