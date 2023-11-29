#include <stdio.h>
#include "line_detector.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define LEFT_SENSOR_PIN 26
#define RIGHT_SENSOR_PIN 11

void LINEDETECTOR_Init()
{
    gpio_init(LEFT_SENSOR_PIN);
    gpio_set_dir(LEFT_SENSOR_PIN, GPIO_IN);
    gpio_pull_up(LEFT_SENSOR_PIN); // Enable pull-up if necessary

    gpio_init(RIGHT_SENSOR_PIN);
    gpio_set_dir(RIGHT_SENSOR_PIN, GPIO_IN);
    gpio_pull_up(RIGHT_SENSOR_PIN); // Enable pull-up if necessary
}

int isLeftSensorLineDetected()
{
    return gpio_get(LEFT_SENSOR_PIN); // Returns 1 if line is detected, 0 otherwise
}

int isRightSensorLineDetected()
{
    return gpio_get(RIGHT_SENSOR_PIN); // Returns 1 if line is detected, 0 otherwise
}