#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pid.h"

PID *PID_create(float kP, float kI, float kD, float setPoint, float min, float max)
{
    PID *newPID = malloc(sizeof(PID));
    if (newPID != NULL)
    {
        newPID->kP = kP; // Proportional
        newPID->kI = kI; // Integral
        newPID->kD = kD; // Deriative
        newPID->setPoint = setPoint;
        newPID->min = min; // Minimum output
        newPID->max = max; // Maximum output
        newPID->p = newPID->i = newPID->d = 0;
    }
    return newPID;
}

//limit the input at the max and min value respectively 
float clampF(float input, float min, float max)
{
    if (input > max)
        return max;
    if (input < min)
        return min;
    return input;
}

void PID_setTarget(PID *pid, float setPoint)
{
    // limiting the setpoint (0.f to 50.f only)
    pid->setPoint = clampF(setPoint, 0.f, 50.f);
}

void PID_setTargetSpeed(PID *pid, int speed)
{
    switch (speed)
    {
    case SPEED_NONE:
        PID_setTarget(pid, 0.f);
        pid->p = pid->i = pid->d = 0.f;
        break;
    case SPEED_LOW:
        PID_setTarget(pid, 25.f);
        break;
    case SPEED_MEDIUM:
        PID_setTarget(pid, 30.f);
        break;
    case SPEED_HIGH:
        PID_setTarget(pid, 35.f);
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
    
    // Proportional
    pid->p = pid->kP * error;

    // Integral
    pid->i += pid->kI * error * deltaTime;
    pid->i = clampF(pid->i, -pid->min, pid->max);

    // Deriative
    pid->d = pid->kD * (error - pid->lastError) / deltaTime;

    float output = pid->p + pid->i + pid->d;
    output = clampF(output, pid->min, pid->max);
    
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