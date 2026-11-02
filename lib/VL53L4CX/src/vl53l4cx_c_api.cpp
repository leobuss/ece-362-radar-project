// vl53l4cx_c_api.cpp - the C++ side of vl53l4cx_c_api.h.
// Holds the one VL53L4CX driver object and forwards each C call to it.

#include "vl53l4cx_c_api.h"
#include "vl53l4cx_class.h"

static TwoWire tof_wire;    // fake Wire, really goes through i2c.c
static VL53L4CX tof_sensor; // st's driver object

int vl53l4cx_c_init(int xshut_pin, uint8_t addr)
{
    tof_sensor.setI2cDevice(&tof_wire);
    tof_sensor.setXShutPin(xshut_pin);
    tof_sensor.begin();                // xshut low (if wired)
    return tof_sensor.InitSensor(addr); // xshut high, wait for boot, load defaults
}

int vl53l4cx_c_set_long_mode(void)
{
    return tof_sensor.VL53L4CX_SetDistanceMode(VL53L4CX_DISTANCEMODE_LONG);
}

int vl53l4cx_c_set_timing_budget_us(uint32_t us)
{
    return tof_sensor.VL53L4CX_SetMeasurementTimingBudgetMicroSeconds(us);
}

int vl53l4cx_c_start(void)
{
    return tof_sensor.VL53L4CX_StartMeasurement();
}

int vl53l4cx_c_stop(void)
{
    return tof_sensor.VL53L4CX_StopMeasurement();
}

int vl53l4cx_c_clear_and_start(void)
{
    return tof_sensor.VL53L4CX_ClearInterruptAndStartMeasurement();
}

int vl53l4cx_c_data_ready(uint8_t *ready)
{
    return tof_sensor.VL53L4CX_GetMeasurementDataReady(ready);
}

int vl53l4cx_c_get_targets(vl53l4cx_c_target_t *targets, int *count)
{
    // static: this struct is fairly large, keep it off the stack
    static VL53L4CX_MultiRangingData_t data;

    *count = 0;
    int status = tof_sensor.VL53L4CX_GetMultiRangingData(&data);
    if (status != 0)
    {
        return status;
    }

    int n = data.NumberOfObjectsFound;
    if (n > VL53L4CX_C_MAX_TARGETS)
    {
        n = VL53L4CX_C_MAX_TARGETS;
    }

    for (int i = 0; i < n; i++)
    {
        targets[i].distance_mm = data.RangeData[i].RangeMilliMeter;
        targets[i].status = data.RangeData[i].RangeStatus;
        targets[i].signal_mcps = data.RangeData[i].SignalRateRtnMegaCps / 65536.0f; // 16.16 fixed point
    }
    *count = n;
    return 0;
}
