// Define a structure to represent accelerometer data
typedef struct
{
    short rawXAxis; // 16-bit signed integer for raw x-axis acceleration
    short rawYAxis; // 16-bit signed integer for raw y-axis acceleration
    short rawZAxis; // 16-bit signed integer for raw z-axis acceleration
} AccelerometerData;

// Declare functions for magnetometer and accelerometer operations
void magnetometerTask(void); // Unused parameter 'params'
void initializeMagnetometer(void);
void initializeAccelerometer(void);
AccelerometerData readAccelerometer(void); // Returns accelerometer data
void initializeCompass(void);
float readCompassDegrees(AccelerometerData acceleration); // Converts accelerometer data to compass degrees