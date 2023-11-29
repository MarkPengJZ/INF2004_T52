#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "magnetometer.h"

// Constants for I2C setup and sensor addresses
#define I2C_BUS i2c0
#define I2C_DATA_RATE 400000   // Data rate in Hz
#define MAGNETOMETER_SCL_PIN 1 // SCL pin number
#define MAGNETOMETER_SDA_PIN 0 // SDA pin number
#define PI 3.14159265358979323846
#define MOVING_AVG_WINDOW_SIZE 10 // Size of the moving average window

float headingBuffer[MOVING_AVG_WINDOW_SIZE];
int bufferIndex = 0;

// Sensor I2C addresses
#define ACCEL_SENSOR_ADDR 0x19  // Accelerometer address
#define MAGNET_SENSOR_ADDR 0x1E // Magnetometer address

// Function to add a value to the buffer and compute the moving average
float computeMovingAverage(float newHeading)
{
    headingBuffer[bufferIndex] = newHeading;
    bufferIndex = (bufferIndex + 1) % MOVING_AVG_WINDOW_SIZE;

    float sum = 0;
    for (int i = 0; i < MOVING_AVG_WINDOW_SIZE; i++)
    {
        sum += headingBuffer[i];
    }
    return sum / MOVING_AVG_WINDOW_SIZE;
}

void magnetometerTask(__unused void *params)
{
    // Initialize the magnetometer once at the start
    initializeMagnetometer();

    // Main loop to continuously read the accelerometer and magnetometer data
    while (true)
    {
        accel_t acceleration = readAccelerometer();
        float compassHeading = readCompassDegrees(acceleration);

        printf("Accel X: %d Accel Y: %d Accel Z: %d Compass: %.2f\n", acceleration.raw_x_axis, acceleration.raw_y_axis, acceleration.raw_z_axis, compassHeading);
    }
}

void initializeMagnetometer(void)
{
    // Initialize I2C with the specified data rate
    i2c_init(I2C_BUS, I2C_DATA_RATE);

    // Configure the SCL and SDA pins for I2C communication
    gpio_set_function(MAGNETOMETER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAGNETOMETER_SDA_PIN, GPIO_FUNC_I2C);

    // Enable internal pull-ups for the I2C pins
    gpio_pull_up(MAGNETOMETER_SCL_PIN);
    gpio_pull_up(MAGNETOMETER_SDA_PIN);

    // Initialize the accelerometer and compass modules
    initializeAccelerometer();
    initializeCompass();
}

void initializeAccelerometer(void)
{
    // Control register addresses and values for accelerometer setup
    const uint8_t CTRL_REG1_A = 0x20;
    const uint8_t ENABLE_ACCEL = 0x57; // 100 Hz data rate, all axes enabled, normal mode
    const uint8_t CTRL_REG4_A = 0x23;
    const uint8_t FULL_SCALE = 0x00; // +/- 2g scale

    // Enable the accelerometer by writing to control register 1
    uint8_t config[] = {CTRL_REG1_A, ENABLE_ACCEL};
    i2c_write_blocking(I2C_BUS, ACCEL_SENSOR_ADDR, config, sizeof(config), true);

    // Set the full scale by writing to control register 4
    config[0] = CTRL_REG4_A;
    config[1] = FULL_SCALE;
    i2c_write_blocking(I2C_BUS, ACCEL_SENSOR_ADDR, config, sizeof(config), true);
}

accel_t readAccelerometer(void)
{
    // Addresses for accelerometer data registers with auto-increment bit set
    const uint8_t OUT_X_L_A = 0x28 | 0x80;
    uint8_t accelData[6] = {0}; // Buffer to hold raw accelerometer data

    // Request accelerometer data
    i2c_write_blocking(I2C_BUS, ACCEL_SENSOR_ADDR, &OUT_X_L_A, 1, true);
    i2c_read_blocking(I2C_BUS, ACCEL_SENSOR_ADDR, accelData, sizeof(accelData), true);

    // Combine high and low bytes for each axis
    int16_t rawXAxis = (int16_t)((accelData[1] << 8) | accelData[0]) >> 4;
    int16_t rawYAxis = (int16_t)((accelData[3] << 8) | accelData[2]) >> 4;
    int16_t rawZAxis = (int16_t)((accelData[5] << 8) | accelData[4]) >> 4;

    // Return raw acceleration data in a struct
    accel_t data = {
        .raw_x_axis = rawXAxis,
        .raw_y_axis = rawYAxis,
        .raw_z_axis = rawZAxis};

    return data;
}

void initializeCompass(void)
{
    // Address and values for magnetometer control registers
    const uint8_t MR_REG_M = 0x02;
    const uint8_t CONTINUOUS_CONVERSION = 0x00;
    const uint8_t CRA_REG_M = 0x00;
    const uint8_t DATA_RATE = 0x10; // 15Hz data rate
    const uint8_t CRB_REG_M = 0x01;
    const uint8_t GAIN = 0x20; // +/- 1.3g scale

    // Enable continuous conversion mode on the magnetometer
    uint8_t config[] = {MR_REG_M, CONTINUOUS_CONVERSION};
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, config, sizeof(config), true);

    // Set the data rate on the magnetometer
    config[0] = CRA_REG_M;
    config[1] = DATA_RATE;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, config, sizeof(config), true);

    // Set the gain on the magnetometer
    config[0] = CRB_REG_M;
    config[1] = GAIN;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, config, sizeof(config), true);
}

float readCompassDegrees(accel_t acceleration)
{
    uint8_t reg[1] = {0};
    uint8_t data[1] = {0};

    // Read 6 bytes of data (msb first)
    // Read xMag msb data from register(0x03)
    reg[0] = 0x03;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_0 = data[0];

    // Read xMag lsb data from register(0x04)
    reg[0] = 0x04;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_1 = data[0];

    // Read yMag msb data from register(0x05)
    reg[0] = 0x07;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_2 = data[0];

    // Read yMag lsb data from register(0x06)
    reg[0] = 0x08;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_3 = data[0];

    // Read zMag msb data from register(0x07)
    reg[0] = 0x05;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_4 = data[0];

    // Read zMag lsb data from register(0x08)
    reg[0] = 0x06;
    i2c_write_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_BUS, MAGNET_SENSOR_ADDR, data, sizeof(data), true);
    uint8_t data1_5 = data[0];

    // Convert the data
    int16_t xMag = (data1_0 << 8) | data1_1;
    int16_t yMag = (data1_2 << 8) | data1_3;
    int16_t zMag = (data1_4 << 8) | data1_5;

    // Convert the accelerometer values to g's
    float ax = acceleration.raw_x_axis / 16384.0; // Assuming +/-2g range, adjust if different
    float ay = acceleration.raw_y_axis / 16384.0;
    float az = acceleration.raw_z_axis / 16384.0;

    // Compute the tilt compensation
    float pitch = asin(-ax);
    float roll = asin(ay / cos(pitch));

    // Tilt-compensated magnetic sensor readings
    float xMagTiltComp = xMag * cos(pitch) + zMag * sin(pitch);
    float yMagTiltComp = xMag * sin(roll) * sin(pitch) + yMag * cos(roll) - zMag * sin(roll) * cos(pitch);

    // Calculate the heading
    float heading = (atan2(yMagTiltComp, xMagTiltComp) * 180.0) / PI;
    if (heading < 0)
    {
        heading += 360;
    }

    // Apply a moving average filter
    return computeMovingAverage(heading);
}

int main(void)
{
    // Initialize the standard library for Raspberry Pi Pico
    stdio_init_all();

    // Start the magnetometer task
    magnetometerTask(NULL);

    return 0;
}
