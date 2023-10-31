#ifndef MOTOR_H_
#define MOTOR_H_

#include "pid.h"

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2

#define MOTOR_DIR_FORWARD 0
#define MOTOR_DIR_REVERSE 1

#define MOTOR_TURN_CLOCKWISE 0
#define MOTOR_TURN_ANTICLOCKWISE 1

// Initialize motor
void MOTOR_init(PID *left, PID *right);

// Set left or right motor's PWM duty cycle.
void MOTOR_setSpeed(uint dutyCycle, int motor);
// Shorthand for MOTOR_setSpeed(0.f, motor);
void MOTOR_stop(int motor);
// Get currently configured duty cycle.
uint MOTOR_getSpeed(int motor);

// Set direction of left or right motor (forward/reverse).
void MOTOR_setDirection(int dir, int motor);
// Get direction of left or right motor.
int MOTOR_getDirection(int motor);

// Move forward by X cm. Blocks until completed.
void MOTOR_moveFoward(int cm);
// Move forward by X cm using PID controller. Non blocking.
void MOTOR_moveFowardPID(PID *pidLeft, PID *pidRight, int cm);
// Move backward by X cm. Blocks until completed.
void MOTOR_moveBackward(int cm);
// Turn on the spot, either clockwise or anticlockwise. Blocks until completed. Angle must be multiple of 45.
void MOTOR_spotTurn(int turnDirection, int angle);
// Turn on the spot using PID. Non blocking. Angle must be multiple of 45.
void MOTOR_spotTurnPID(PID *pidLeft, PID *pidRight, int turnDirection, int angle);

// Helper function to convert from CM to number of interrupts.
int CMtoSteps(float cm);
#endif