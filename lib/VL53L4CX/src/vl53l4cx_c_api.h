// vl53l4cx_c_api.h - plain C functions wrapped around ST's C++ VL53L4CX driver,
// so src/i2c.c (a .c file) can use the driver. Every function returns 0 on success
// or a negative ST error code (see vl53l4cx_error_codes.h).

#ifndef VL53L4CX_C_API_H
#define VL53L4CX_C_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VL53L4CX_C_MAX_TARGETS 4   // the sensor reports up to 4 objects per measurement

typedef struct
{
    int16_t distance_mm;   // distance to this object in mm
    uint8_t status;        // st range status, 0 = valid (see vl53l4cx_def.h)
    float signal_mcps;     // return signal strength, higher = more trustworthy
} vl53l4cx_c_target_t;

// power-cycle the sensor with xshut (pass -1 if xshut isn't wired), wait for boot and load
// st's default settings. addr is the 8-bit address st uses (0x52 = 0x29 << 1).
int vl53l4cx_c_init(int xshut_pin, uint8_t addr);

// long distance mode, needed for the full ~6 m range
int vl53l4cx_c_set_long_mode(void);

// time spent on each measurement. longer = more range and less noise, but slower
int vl53l4cx_c_set_timing_budget_us(uint32_t us);

int vl53l4cx_c_start(void);
int vl53l4cx_c_stop(void);

// clear the "data ready" flag and start the next measurement
int vl53l4cx_c_clear_and_start(void);

// *ready = 1 when a finished measurement is waiting to be read
int vl53l4cx_c_data_ready(uint8_t *ready);

// copy out every object from the finished measurement (*count of them)
int vl53l4cx_c_get_targets(vl53l4cx_c_target_t *targets, int *count);

#ifdef __cplusplus
}
#endif

#endif
