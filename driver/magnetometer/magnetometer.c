#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>

#define GY511_I2C_PORT i2c0
#define GY511_BAUD_RATE 100000
#define GY511_SDA_PIN 0
#define GY511_SCL_PIN 1

#define GY511_ACCEL_ADDR 0x19
#define GY511_MAG_ADDR 0x1E

// GY511 Accelerometer Register Addresses
#define CTRL_REG1_A 0x20
#define CTRL_REG1_A_CONFIG 0x37
#define CTRL_REG4_A 0x23
#define CTRL_REG4_A_CONFIG 0x00 // Full-scale selection: 2g, High resolution disabled
#define ACC_XYZ_REG 0xA8 // 0x28 (Acc Output register) + 0x80 (auto increment)

// GY511 Magnetometer Register Addresses
#define CRA_REG_M 0x00
#define CRA_REG_M_CONFIG 0x10 // 15 Hz, normal mode
#define CRB_REG_M 0x01
#define CRB_REG_M_CONFIG 0xE0 // gause +- 8.1, gain 230, gain Z 205
#define MR_REG_M 0x02
#define MR_REG_M_CONFIG 0x00 // Continuous conversion mode
#define MAG_XYZ_REG 0x03 // 0x03 (Mag Output register) + 0x0A (auto increment)

uint8_t get_Heading_in_Degrees(int16_t x, int16_t y)
{
    int8_t angle = atan2(y, x) * 180 / M_PI;
    return angle;
}

void writeI2CRegister(uint8_t addr, uint8_t writeAddr, uint8_t writeData)
{
    uint8_t data[2] = {writeAddr, writeData};
    i2c_write_blocking(GY511_I2C_PORT, addr, data, 2, true);
}


/* TEST features the printed version of X Y Z values */
void readI2C_XYZ_TEST(uint8_t addr, uint8_t reg)
{
    int16_t x, y, z;
    uint8_t data[6];

    // Read the XYZ values
    i2c_write_blocking(GY511_I2C_PORT, addr, &reg, 1, true);
    i2c_read_blocking(GY511_I2C_PORT, addr, data, 6, false);

    // Combine the low and high bytes to get the values
    x = (data[1] << 8) | data[0];
    y = (data[3] << 8) | data[2];
    z = (data[5] << 8) | data[4];

    if (addr == GY511_MAG_ADDR)
    {
        // The magnetometer is in 2's complement form and needs to be converted to magnetic field values.
        int8_t angle = atan2(y, x) * 180 / M_PI;
        
        printf("Magnetic Field (X, Y, Z): %d, %d, %d\t Heading (in degrees): %d\t", x, y, z, angle);
    }
    else if (addr == GY511_ACCEL_ADDR)
    {
        // The accelerometer is in 2's complement form and needs to be converted to Gs.
        printf("Acceleration (X, Y, Z):  %d, %d, %d\t", x, y, z);
    }
}

void readI2C_XYZ(uint16_t XYZ[3] , uint8_t addr, uint8_t reg)
{
    uint8_t buffer[6];

    // Read the XYZ values
    i2c_write_blocking(GY511_I2C_PORT, addr, &reg, 1, true);
    i2c_read_blocking(GY511_I2C_PORT, addr, buffer, 6, false);

    // Combine the low and high bytes to get the values
    XYZ[0] = (buffer[1] << 8) | buffer[0];
    XYZ[1] = (buffer[3] << 8) | buffer[2];
    XYZ[2] = (buffer[5] << 8) | buffer[4];

    if (addr == GY511_MAG_ADDR)
    {
        // The magnetometer is in 2's complement form and needs to be converted to magnetic field values.
        

    }
    else if (addr == GY511_ACCEL_ADDR)
    {
        // The accelerometer is in 2's complement form and needs to be converted to Gs.
  
    }
}

void accelerometer_init()
{
    // Initialize the accelerometer
    writeI2CRegister(GY511_ACCEL_ADDR, CTRL_REG1_A, CTRL_REG1_A_CONFIG);
    writeI2CRegister(GY511_ACCEL_ADDR, CTRL_REG4_A, CTRL_REG4_A_CONFIG);
    
}

void magnetometer_init()
{   
    // Initialize the magnetometer
    writeI2CRegister(GY511_MAG_ADDR, CRA_REG_M, CRA_REG_M_CONFIG);
    writeI2CRegister(GY511_MAG_ADDR, CRB_REG_M, CRB_REG_M_CONFIG);
    writeI2CRegister(GY511_MAG_ADDR, MR_REG_M, MR_REG_M_CONFIG);
}

void GY511_init()     
{   
    sleep_ms(1000); // Wait for a second to let the sensor initialize
    i2c_init(GY511_I2C_PORT, GY511_BAUD_RATE); // Initialize I2C at 9600 kHz

    gpio_set_function(GY511_SDA_PIN, GPIO_FUNC_I2C); // SDA
    gpio_set_function(GY511_SCL_PIN, GPIO_FUNC_I2C); // SCL
    gpio_pull_up(GY511_SDA_PIN);                     // SDA
    gpio_pull_up(GY511_SCL_PIN);                     // SCL

    accelerometer_init();
    magnetometer_init();
}

int main()
{
    stdio_init_all();
    GY511_init();

    while (1)
    {   
        readI2C_XYZ_TEST(GY511_ACCEL_ADDR, ACC_XYZ_REG ); // Read accelerometer data
        readI2C_XYZ_TEST(GY511_MAG_ADDR, MAG_XYZ_REG);   // Read magnetometer data
        printf("\n");
        sleep_ms(1000); // Delay for 1 second before reading again
    }

    return 0;
}