#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pid.h"

/**
 * Creates a new PID controller instance.
 *
 * @param kP Proportional gain
 * @param kI Integral gain
 * @param kD Derivative gain
 * @param setPoint Desired value
 * @param min Minimum output
 * @param max Maximum output
 * @return Pointer to the newly created PID controller instance, or NULL if memory allocation fails
 */
PID *PID_create(float kP, float kI, float kD, float setPoint, float min, float max)
{
    // Allocate memory for the PID controller instance
    PID *newPID = malloc(sizeof(PID));

    // Check if memory allocation was successful
    if (newPID != NULL)
    {
        // Initialize PID controller parameters
        newPID->kP = kP;
        newPID->kI = kI;
        newPID->kD = kD;
        newPID->setPoint = setPoint;
        newPID->min = min;
        newPID->max = max;

        // Initialize error terms to zero
        newPID->p = 0.0f;
        newPID->i = 0.0f;
        newPID->d = 0.0f;
    }

    // Return the newly created PID controller instance
    return newPID;
}

/**
 * Limits a float value to the specified minimum and maximum values.
 *
 * @param input Value to be limited
 * @param min Minimum value
 * @param max Maximum value
 * @return The limited input value
 */
float clampF(float input, float min, float max)
{
    // Check if input value exceeds the maximum
    if (input > max)
    {
        // Replace input value with the maximum
        return max;
    }

    // Check if input value falls below the minimum
    if (input < min)
    {
        // Replace input value with the minimum
        return min;
    }

    // Input value is within the specified range, return it unmodified
    return input;
}

/**
 * Sets the target value for the PID controller.
 *
 * @param pid PID controller instance
 * @param setPoint Target value (0.0f to 50.0f)
 */
void PID_setTarget(PID *pid, float setPoint)
{
    // Limit the setpoint to the allowed range (0.0f to 50.0f)
    pid->setPoint = clampF(setPoint, 0.0f, 50.0f);
}

/**
 * Sets the target speed for the PID controller using predefined speed constants.
 *
 * @param pid PID controller instance
 * @param speed Predefined speed constant: SPEED_NONE, SPEED_LOW, SPEED_MEDIUM, or SPEED_HIGH
 */
void PID_setTargetSpeed(PID *pid, int speed)
{
    // Set the target value based on the specified speed constant
    switch (speed)
    {
    case SPEED_NONE:
        PID_setTarget(pid, 0.0f);        // Stop the motor
        pid->p = pid->i = pid->d = 0.0f; // Reset error terms
        break;

    case SPEED_LOW:
        PID_setTarget(pid, 25.0f);
        break;

    case SPEED_MEDIUM:
        PID_setTarget(pid, 30.0f);
        break;

    case SPEED_HIGH:
        PID_setTarget(pid, 35.0f);
        break;
    }
}

/**
 * Calculates and applies PID control to the motor.
 *
 * @param pid PID controller instance
 * @param input Current motor speed
 * @param deltaTime Time in seconds since the last PID calculation
 * @return PWM duty cycle in percentage (0-100)
 */
uint PID_run(PID *pid, float input, float deltaTime)
{
    // Calculate the current error
    float error = pid->setPoint - input;

    // Proportional term
    pid->p = pid->kP * error;

    // Integral term
    pid->i += pid->kI * error * deltaTime;
    pid->i = clampF(pid->i, -pid->min, pid->max); // Limit integral term to avoid runaway

    // Derivative term
    pid->d = pid->kD * (error - pid->lastError) / deltaTime;

    // Calculate the overall PID output
    float output = pid->p + pid->i + pid->d;
    output = clampF(output, pid->min, pid->max); // Limit overall output to avoid saturation

    // Apply hysteresis to avoid motor oscillation
    if (output > 0)
    {
        // Apply 0.4f gain to the PID output to achieve faster response
        return 65 + roundf(output * 0.4f); // Transform output to 60-100% range for the motor
    }
    else
    {
        // Return zero duty cycle when the motor should not be moving
        return 0;
    }
}