#ifndef MOTOR_H_
#define MOTOR_H_

#include "pid.h"

#define LEFT_MOTOR 1
#define RIGHT_MOTOR 2

#define DIRECTION_FORWARD 0
#define DIRECTION_REVERSE 1

#define TURN_CLOCKWISE 0
#define TURN_COUNTERCLOCKWISE 1

// Initialize motor
void initializeMotor(PID *left, PID *right);

// Set left or right motor's PWM duty cycle.
void setMotorSpeed(uint dutyCycle, int motor);
// Shorthand for setMotorSpeed(0.f, motor);
void haltMotor(int motor);
// Get currently configured duty cycle.
uint retrieveMotorSpeed(int motor);

// Set direction of left or right motor (forward/reverse).
void setMotorDirection(int dir, int motor);
// Get direction of left or right motor.
int retrieveMotorDirection(int motor);

// Move forward by X cm. Blocks until completed.
void moveMotorForward(int cm);
// Move forward by X cm using PID controller. Non blocking.
void moveMotorForwardPID(PID *pidLeft, PID *pidRight, int cm);
// Move backward by X cm. Blocks until completed.
void moveMotorBackward(int cm);
// Turn on the spot, either clockwise or anticlockwise. Blocks until completed. Angle must be multiple of 45.
void MOTOR_spotTurn(int turnDirection, int angle);
// Turn on the spot using PID. Non blocking. Angle must be multiple of 45.
void MOTOR_spotTurnPID(PID *pidLeft, PID *pidRight, int turnDirection, int angle);

// Helper function to convert from CM to number of interrupts.
int CMtoSteps(float cm);
#endif