//  This is for the Arduino's Wire (I2C) library so ST's VL53L4CX driver builds with the Pico SDK.
// This class does not touch the hardware itself. It just collects bytes the same way Arduino's
// Wire does and hands them to tof_bus_write() / tof_bus_read() in src/i2c.c, which run the
// actual RP2350 I2C transfers.

#ifndef VL53L4CX_WIRE_SHIM_H
#define VL53L4CX_WIRE_SHIM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// defined in src/i2c.c (return 0 on success)
extern "C" int tof_bus_write(uint8_t addr, const uint8_t *src, size_t len, bool keep_bus);
extern "C" int tof_bus_read(uint8_t addr, uint8_t *dst, size_t len);

class TwoWire
{
public:
    void begin(void) {}
    void end(void) {}

    // start building a write to the 7-bit address addr
    void beginTransmission(uint8_t addr)
    {
        tx_addr = addr;
        tx_len = 0;
    }

    size_t write(uint8_t b)
    {
        return write(&b, 1);
    }

    // queue bytes, returns 0 if they don't fit (st's driver treats 0 as an error)
    size_t write(const uint8_t *data, size_t n)
    {
        if (tx_len + n > sizeof(tx_buf))
        {
            return 0;
        }
        memcpy(&tx_buf[tx_len], data, n);
        tx_len += n;
        return n;
    }

    // send the queued bytes. stop = false keeps the bus (repeated start) so a read can follow.
    // returns 0 on success like arduino, 4 ("other error") on failure
    uint8_t endTransmission(bool stop = true)
    {
        return tof_bus_write(tx_addr, tx_buf, tx_len, !stop) == 0 ? 0 : 4;
    }

    // read n bytes from the device into rx_buf, returns how many were read
    uint8_t requestFrom(uint8_t addr, uint8_t n)
    {
        rx_len = 0;
        rx_pos = 0;
        if (n > sizeof(rx_buf))
        {
            n = sizeof(rx_buf);
        }
        if (tof_bus_read(addr, rx_buf, n) == 0)
        {
            rx_len = n;
        }
        return rx_len;
    }

    int available(void)
    {
        return rx_len - rx_pos;
    }

    int read(void)
    {
        if (rx_pos >= rx_len)
        {
            return -1;
        }
        return rx_buf[rx_pos++];
    }

private:
    uint8_t tx_addr = 0;
    uint8_t tx_buf[64];   // st's driver sends 2 index bytes + up to 32 data bytes at a time
    size_t tx_len = 0;
    uint8_t rx_buf[64];
    int rx_len = 0;
    int rx_pos = 0;
};

#endif
