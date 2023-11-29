#ifndef ENCODER_H_
#define ENCODER_H_

// Encoder identifiers
#define ENCODER_LEFT 1
#define ENCODER_RIGHT 2

// Initialize encoder pins and interrupts
void ENCODER_init(void);

// Get current wheel speed in revolutions per second (RPS)
float ENCODER_getWheelSpeed(int encoder);

// Get current wheel interrupt speed in interrupts per second (IPS)
int ENCODER_getWheelInterruptSpeed(int encoder);

// Block until a specified number of interrupt counts have been reached
void ENCODER_waitForISRInterrupts(uint target);

// Set a callback function to be called after a specified number of interrupt counts have been reached
void ENCODER_alertAfterISRInterrupts(uint target, repeating_timer_callback_t callback);

// Convert a distance in centimeters (cm) to encoder steps
int ENCODER_cmToSteps(float cm);

#endif