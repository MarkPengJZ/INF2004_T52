#ifndef MOTOR_H_
#define MOTOR_H_

#include "pid.h"

// Define motor constants
#define MOTOR_LEFT 1  // Represents the left motor
#define MOTOR_RIGHT 2 // Represents the right motor

#define MOTOR_DIR_FORWARD 0 // Motor direction: forward
#define MOTOR_DIR_REVERSE 1 // Motor direction: reverse

#define MOTOR_TURN_CLOCKWISE 0     // Rotation direction: clockwise
#define MOTOR_TURN_ANTICLOCKWISE 1 // Rotation direction: anticlockwise

// Initialize the motors
void MOTOR_init(PID *leftPID, PID *rightPID); // Initializes the PID controllers for both motors

// Set the PWM duty cycle for a specific motor
void MOTOR_setSpeed(uint dutyCycle, int motor); // Sets the PWM duty cycle for the specified motor (left or right)

// Convenience function to stop a motor by setting its PWM duty cycle to zero
void MOTOR_stop(int motor); // Shorthand for MOTOR_setSpeed(0.f, motor)

// Get the currently configured PWM duty cycle for a specified motor
uint MOTOR_getSpeed(int motor); // Retrieves the current PWM duty cycle for the specified motor (left or right)

// Set the direction of rotation for a specified motor
void MOTOR_setDirection(int dir, int motor); // Sets the direction of rotation (forward or reverse) for the specified motor (left or right)

// Get the current direction of rotation for a specified motor
int MOTOR_getDirection(int motor); // Retrieves the current direction of rotation (forward or reverse) for the specified motor (left or right)

// Move both motors forward a specified distance in centimeters. Blocks until the movement is complete.
void MOTOR_moveFoward(int cm); // Moves both motors forward the specified distance (blocking)

// Move both motors forward a specified distance in centimeters using PID control. Non-blocking.
void MOTOR_moveFowardPID(PID *pidLeft, PID *pidRight, int cm); // Initiates a forward movement using PID control (non-blocking)

// Move both motors backward a specified distance in centimeters. Blocks until the movement is complete.
void MOTOR_moveBackward(int cm); // Moves both motors backward the specified distance (blocking)

// Perform a spot turn in place by the specified angle in degrees. The angle must be a multiple of 45. Blocks until the turn is complete.
void MOTOR_spotTurn(int turnDirection, int angle); // Performs a spot turn (blocking)

// Perform a spot turn in place using PID control by the specified angle in degrees. The angle must be a multiple of 45. Non-blocking.
void MOTOR_spotTurnPID(PID *pidLeft, PID *pidRight, int turnDirection, int angle); // Initiates a spot turn using PID control (non-blocking)

// Convert a distance in centimeters to the corresponding number of motor interrupts
int CMtoSteps(float cm); // Converts a distance in centimeters to the corresponding number of motor interrupts
#endif