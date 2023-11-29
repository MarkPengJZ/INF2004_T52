
typedef struct
{
    int16_t raw_x_axis;
    int16_t raw_y_axis;
    int16_t raw_z_axis;
} accel_t;

void magnetometer_task(__unused void *params);
void initializeMagnetometer(void);
void initializeAccelerometer(void);
accel_t readAccelerometer(void);
void initializeCompass(void);
float readCompassDegrees(accel_t acceleration);