#ifndef I2C_H
#define I2C_H
 
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
 
#define TOF_NO_TARGET -1      // nothing in range
#define TOF_ERROR     -2      // sensor or i2c problem
#define TOF_MAX_RANGE_MM 6000 // use to scale the radar display
 
// vl53l4cx distance sensor
bool tof_init(void);
int tof_read_distance_mm(void);
 
// raw i2c (used by the library in lib/)
int tof_bus_write(uint8_t addr, const uint8_t *src, size_t len, bool keep_bus);
int tof_bus_read(uint8_t addr, uint8_t *dst, size_t len);
 
// test
void i2c_test(void);
 
#endif
 