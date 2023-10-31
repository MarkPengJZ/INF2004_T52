#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pid.h"

PID *PID_create(float kP, float kI, float kD, float setPoint, float min, float max)
{
    PID *createdPID = malloc(sizeof(PID));
    if (createdPID != NULL)
    {
        createdPID->kP = kP; // Proportional gain
        createdPID->kI = kI; // Integral gain
        createdPID->kD = kD; // Derivative gain
        createdPID->setPoint = setPoint;
        createdPID->min = min; // Minimum output
        createdPID->max = max; // Maximum output
        createdPID->p = createdPID->i = createdPID->d = 0;
    }
    return createdPID;
}

// Constrain the input value at the max and min value respectively
float limitValueInRange(float input, float min, float max)
{
    if (input > max)
        return max;
    if (input < min)
        return min;
    return input;
}

void setPIDTargetValue(PID *pid, float setPoint)
{
    // Constrain the setpoint value (0.f to 50.f only)
    pid->setPoint = limitValueInRange(setPoint, 0.f, 50.f);
}

void setPIDTargetValueSpeed(PID *pid, int speed)
{
    switch (speed)
    {
    case NO_SPEED:
        setPIDTargetValue(pid, 0.f);
        pid->p = pid->i = pid->d = 0.f;
        break;
    case LOW_SPEED:
        setPIDTargetValue(pid, 25.f);
        break;
    case MEDIUM_SPEED:
        setPIDTargetValue(pid, 30.f);
        break;
    case HIGH_SPEED:
        setPIDTargetValue(pid, 35.f);
        break;
    }
}

/// @brief PID Controller
/// @param pid PID controller data
/// @param input Current value
/// @param deltaTime Time in seconds since last controller call
/// @return PWM duty cycle in percentage (0-100)
uint PID_run(PID *pid, float input, float deltaTime)
{
    float error = pid->setPoint - input;

    // Proportional gain
    pid->p = pid->kP * error;

    // Integral gain
    pid->i += pid->kI * error * deltaTime;
    pid->i = limitValueInRange(pid->i, -pid->min, pid->max);

    // Derivative gain
    pid->d = pid->kD * (error - pid->lastError) / deltaTime;

    float output = pid->p + pid->i + pid->d;
    output = limitValueInRange(output, pid->min, pid->max);

    pid->lastError = error;

    // Motors needs certain % of duty cycle before it has enough torque to start moving.
    // Normalize the speed so that the original PID output of 0-100% is transformed into 60%-100%.
    if (output > 0)
    {
        return 60 + roundf(output * 0.4f);
    }
    else
    {
        return 0;
    }
}