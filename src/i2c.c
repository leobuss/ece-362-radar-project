#include "header.h"
#include "hardware/i2c.h"
#include "i2c.h"
#include "vl53l4cx_c_api.h"
 
// pins (sda/scl must be a matching i2c pair)
#define TOF_I2C   i2c0
#define TOF_SDA   8
#define TOF_SCL   9
#define TOF_XSHUT -1       // sensor on/off pin, -1 if tied high on the breakout
 
#define TOF_ADDR  0x29     // sensor's 7-bit i2c address
#define TOF_BUDGET_US 50000 // time per measurement (33000 to 200000), longer = more range
#define TOF_TIMEOUT_MS 500  // give up if a measurement takes longer than this
 
 
// ---------- raw i2c  ----------
 
// write len bytes. keep_bus = true means no stop bit, so a read can follow
int tof_bus_write(uint8_t addr, const uint8_t *src, size_t len, bool keep_bus)
{
    int n = i2c_write_timeout_us(TOF_I2C, addr, src, len, keep_bus, 1000 + len * 100);
    return (n == (int)len) ? 0 : -1;
}
 
// read len bytes
int tof_bus_read(uint8_t addr, uint8_t *dst, size_t len)
{
    int n = i2c_read_timeout_us(TOF_I2C, addr, dst, len, false, 1000 + len * 100);
    return (n == (int)len) ? 0 : -1;
}
 
 
// ---------- sensor ----------
 
// set up i2c and the sensor, returns true if it worked
bool tof_init(void)
{
    // i2c0 at 400 khz
    i2c_init(TOF_I2C, 400000);
    gpio_set_function(TOF_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TOF_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TOF_SDA);
    gpio_pull_up(TOF_SCL);
 
    // turn sensor on
    if (TOF_XSHUT >= 0)
    {
        gpio_init(TOF_XSHUT);
        gpio_set_dir(TOF_XSHUT, GPIO_OUT);
        gpio_put(TOF_XSHUT, 1);
        sleep_ms(2);
    }
 
    // read the 2 byte model id at register 0x010F, should be 0xEB 0xAA
    // (catches bad wiring, otherwise st's driver would hang)
    uint8_t reg[2] = {0x01, 0x0F};
    uint8_t id[2] = {0, 0};
    if (tof_bus_write(TOF_ADDR, reg, 2, true) != 0 || tof_bus_read(TOF_ADDR, id, 2) != 0)
    {
        printf("tof: no response, check wiring\n");
        return false;
    }
    if (id[0] != 0xEB || id[1] != 0xAA)
    {
        printf("tof: wrong id 0x%02X%02X\n", id[0], id[1]);
        return false;
    }
 
    // st's driver: boot sensor, long range mode (6 m), set timing, start
    if (vl53l4cx_c_init(TOF_XSHUT, TOF_ADDR << 1) != 0 ||
        vl53l4cx_c_set_long_mode() != 0 ||
        vl53l4cx_c_set_timing_budget_us(TOF_BUDGET_US) != 0 ||
        vl53l4cx_c_start() != 0)
    {
        printf("tof: setup failed\n");
        return false;
    }
 
    tof_read_distance_mm(); // first result after starting is junk, throw it away
    printf("tof: ready\n");
    return true;
}
 
// take one new measurement, return distance to the closest object in mm
// returns TOF_NO_TARGET if nothing seen, TOF_ERROR if something broke
// (starts measuring when called, so move the servo first)
int tof_read_distance_mm(void)
{
    vl53l4cx_c_target_t targets[VL53L4CX_C_MAX_TARGETS];
    int count = 0;
    uint8_t ready = 0;
 
    // start a new measurement
    if (vl53l4cx_c_clear_and_start() != 0)
    {
        return TOF_ERROR;
    }
 
    // wait for it to finish
    for (int ms = 0; !ready; ms++)
    {
        if (ms > TOF_TIMEOUT_MS || vl53l4cx_c_data_ready(&ready) != 0)
        {
            printf("tof: timeout\n");
            return TOF_ERROR;
        }
        sleep_ms(1);
    }
 
    // get results (the sensor can see up to 4 objects)
    if (vl53l4cx_c_get_targets(targets, &count) != 0)
    {
        return TOF_ERROR;
    }
 
    // keep the closest valid one (status 0, 6, 11 = good reading)
    int closest = TOF_NO_TARGET;
    for (int i = 0; i < count; i++)
    {
        uint8_t s = targets[i].status;
        if (s != 0 && s != 6 && s != 11)
        {
            continue;
        }
        if (closest == TOF_NO_TARGET || targets[i].distance_mm < closest)
        {
            closest = targets[i].distance_mm;
        }
    }
    return closest;
}
 
 
// ---------- test ----------
 
// print distance over and over
void i2c_test(void)
{
    if (!tof_init())
    {
        return;
    }
    for (;;)
    {
        printf("distance: %d mm\n", tof_read_distance_mm());
    }
}