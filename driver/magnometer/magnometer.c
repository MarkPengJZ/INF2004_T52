#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {
    stdio_init_all();

    // Initialize I2C 0
    i2c_init(i2c0, 100000);
    gpio_set_function(8, GPIO_FUNC_I2C);
    gpio_set_function(9, GPIO_FUNC_I2C);
    gpio_pull_up(8);
    gpio_pull_up(9);

    // Initialize the GY-511 (HMC5883L) sensor
    uint8_t setup[] = {0x02, 0x00};
    i2c_write_blocking(i2c0, 0x1E, setup, sizeof(setup), false);

    while (1) {
        uint8_t data[6];

        // Read magnetometer data (x, z, y) in 2's complement format
        uint8_t register_byte = 0x03;
        i2c_write_blocking(i2c0, 0x1E, &register_byte, 1, true);
        i2c_read_blocking(i2c0, 0x1E, data, sizeof(data), false);

        // Convert data to individual values (16-bit signed integers)
        int16_t x = (data[0] << 8) | data[1];
        int16_t z = (data[2] << 8) | data[3];
        int16_t y = (data[4] << 8) | data[5];

        printf("Magnetometer Data (X, Z, Y): %d, %d, %d\n", x, z, y);
        sleep_ms(1000);
    }

    return 0;
}
