#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define IR_SENSOR_PIN 26 // Replace with the actual GPIO pin

int main() {
    stdio_init_all();

    // Set up the GPIO pin for the IR sensor as input
    gpio_init(IR_SENSOR_PIN);
    gpio_set_dir(IR_SENSOR_PIN, GPIO_IN);

    uint32_t start_time = 0;
    uint32_t pulse_width = 0;
    bool is_high = false;

    while (1) {
        if (gpio_get(IR_SENSOR_PIN)) {
            if (!is_high) {
                start_time = time_us_32();
                is_high = true;
            }
        } else {
            if (is_high) {
                pulse_width = time_us_32() - start_time;
                printf("Pulse width: %lu us\n", pulse_width); // higher value means line is thicker
                is_high = false;
            }
        }
    }

    return 0;
}
