// Include necessary libraries
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "magnetometer.h"

// Define constants for I2C communication and sensor addresses
#define I2C_BUS i2c0
#define I2C_DATA_RATE 400000   // Data rate in Hz
#define MAGNETOMETER_SCL_PIN 1 // SCL pin number
#define MAGNETOMETER_SDA_PIN 0 // SDA pin number
#define PI 3.14159265358979323846
#define MOVING_AVG_WINDOW_SIZE 10 // Size of the moving average window

// Declare variables for storing sensor data and moving average
float headingBuffer[MOVING_AVG_WINDOW_SIZE];
int bufferIndex = 0;

// Define sensor I2C addresses
#define ACCEL_SENSOR_ADDR 0x19  // Accelerometer address
#define MAGNET_SENSOR_ADDR 0x1E // Magnetometer address

// Function to calculate the moving average of compass readings
float computeMovingAverage(float newHeading)
{
    // Add new heading to the buffer and update the index
    headingBuffer[bufferIndex] = newHeading;
    bufferIndex = (bufferIndex + 1) % MOVING_AVG_WINDOW_SIZE;

    // Calculate the sum of all readings in the buffer
    float sum = 0;
    for (int i = 0; i < MOVING_AVG_WINDOW_SIZE; i++)
    {
        sum += headingBuffer[i];
    }

    // Calculate and return the moving average
    return sum / MOVING_AVG_WINDOW_SIZE;
}

// Main function that runs continuously
int main(void)
{
    // Initialize standard library for Raspberry Pi Pico
    stdio_init_all();

    // Initialize the magnetometer and accelerometer
    initializeMagnetometer();
    initializeAccelerometer();

    // Continuously read sensor data and calculate compass heading
    while (true)
    {
        // Read accelerometer data
        accel_t acceleration = readAccelerometer();

        // Calculate compass heading with tilt compensation and moving average
        float compassHeading = readCompassDegrees(acceleration);

        // Display sensor data and compass heading
        printf("Accel X: %d Accel Y: %d Accel Z: %d Compass: %.2f\n",
               acceleration.raw_x_axis, acceleration.raw_y_axis, acceleration.raw_z_axis, compassHeading);
    }
}