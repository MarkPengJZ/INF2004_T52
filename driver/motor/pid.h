#ifndef PID_H_
#define PID_H_

#define NO_SPEED 0
#define LOW_SPEED 1
#define MEDIUM_SPEED 2
#define HIGH_SPEED 3

#define uint unsigned int

// Memory structure for PID controller
typedef struct _PID {
    float kP, kI, kD;
    float setPoint;
    float min, max;

    float p, i, d;
    float lastError;
} PID;

// Instantiate a new PID controller
PID* createPIDController(float kP, float kI, float kD, float setPoint, float min, float max);
// Update the setpoint for the PID controller for a PID controller.
void setPIDTargetValue(PID *pid, float setPoint);
// Shorthand for setPIDTargetValue using the NO_SPEED, MEDIUM_SPEED, etc for predefined speeds.
void setPIDTargetValueSpeed(PID *pid, int speed);
// Execute the PID control algorithm
uint executePIDControlLoop(PID* pid, float input, float deltaTime);

#endif