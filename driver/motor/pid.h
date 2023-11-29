// Header guard to prevent multiple inclusions
#ifndef PID_H_
#define PID_H_

// Define predefined speed constants
#define SPEED_NONE 0
#define SPEED_LOW 1
#define SPEED_MEDIUM 2
#define SPEED_HIGH 3

// Define alias for unsigned integer type
typedef unsigned int uint;

// Structure to represent PID controller parameters and state
typedef struct _PID
{
    float kP, kI, kD; // Proportional, integral, and derivative gains
    float setPoint;   // Desired value
    float min, max;   // Output range
    float p, i, d;    // Proportional, integral, and derivative error terms
    float lastError;  // Previous error value
} PID;

// Function to create a new PID controller instance
PID *PID_create(float kP, float kI, float kD, float setPoint, float min, float max);

// Function to set the target value for the PID controller
void PID_setTarget(PID *pid, float setPoint);

// Convenience function to set the target value using predefined speed constants
void PID_setTargetSpeed(PID *pid, int speed);

// Function to perform PID control calculation
uint PID_run(PID *pid, float input, float deltaTime);

#endif