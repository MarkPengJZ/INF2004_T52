#ifndef PID_H_
#define PID_H_

#define SPEED_NONE 0
#define SPEED_LOW 1
#define SPEED_MEDIUM 2
#define SPEED_HIGH 3

#define uint unsigned int

// PID controller memory
typedef struct _PID {
    float kP, kI, kD;
    float setPoint;
    float min, max;

    float p, i, d;
    float lastError;
} PID;

// Create a new PID controller
PID* PID_create(float kP, float kI, float kD, float setPoint, float min, float max);
// Set the new setpoint for a PID controller.
void PID_setTarget(PID *pid, float setPoint);
// Shorthand for PID_setTarget using the SPEED_NONE, SPEED_MEDIUM, etc for predefined speeds.
void PID_setTargetSpeed(PID *pid, int speed);
// PID control loop
uint PID_run(PID* pid, float input, float deltaTime);

#endif