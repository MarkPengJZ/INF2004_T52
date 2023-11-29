#ifndef LINE_DETECTOR_H
#define LINE_DETECTOR_H

#define MOVING_AVERAGE_WINDOW 10

// Function to initialize the line detector
void LINEDETECTOR_Init(void);

// Function to update the moving average and check for line detection
int isLeftSensorLineDetected(void);
int isRightSensorLineDetected(void);

#endif // LINE_DETECTOR_H
