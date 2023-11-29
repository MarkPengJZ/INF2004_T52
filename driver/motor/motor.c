#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "motor.h"
#include "pid.h"
#include "../encoder/encoder.h"

// Pin definitions for motor control
#define GPIO_PIN_PWM_EN1 14  // Enable pin for left motor PWM
#define GPIO_PIN_PWM_EN2 15  // Enable pin for right motor PWM
#define GPIO_PIN_MOTOR_IN1 8 // Input pin for left motor direction (IN1)
#define GPIO_PIN_MOTOR_IN2 9 // Input pin for left motor direction (IN2)
#define GPIO_PIN_MOTOR_IN3 6 // Input pin for right motor direction (IN3)
#define GPIO_PIN_MOTOR_IN4 7 // Input pin for right motor direction (IN4)

// Define PWM cycle period
#define PWM_CYCLE 254

// Define minimum angle for spot turns
#define MIN_TURN_ANGLE 45

static uint pwm_slice;
// Global variables for motor state
static uint8_t motorSpeedLeft = 0;  // Current speed of the left motor
static uint8_t motorSpeedRight = 0; // Current speed of the right motor
static int motorDirLeft = -1;       // Current direction of the left motor (-1: forward, 1: reverse)
static int motorDirRight = -1;      // Current direction of the right motor (-1: forward, 1: reverse)

// PID controllers for left and right motors
static PID *pidLeft;
static PID *pidRight;

// Initialize motor controllers and hardware
void MOTOR_init(PID *left, PID *right)
{
    printf("[Motor] Initialization start\n");

    // Initialize GPIO pins for motor control
    gpio_init(GPIO_PIN_MOTOR_IN1);
    gpio_init(GPIO_PIN_MOTOR_IN2);
    gpio_init(GPIO_PIN_MOTOR_IN3);
    gpio_init(GPIO_PIN_MOTOR_IN4);
    gpio_set_dir(GPIO_PIN_MOTOR_IN1, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN2, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN3, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN4, GPIO_OUT);

    // Configure PWM pins for motor control
    gpio_set_function(GPIO_PIN_PWM_EN1, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_PIN_PWM_EN2, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN1);
    assert(pwm_slice == pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN2));

    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&c, 5);       // Set PWM clock divider
    pwm_config_set_wrap(&c, PWM_CYCLE - 1); // Set PWM cycle period

    // Set initial motor state
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    pwm_init(pwm_slice, &c, true);

    pidLeft = left;
    pidRight = right;

    printf("[Motor] Initialization complete\n");
}

// Set motor speed (range: 0-100%)
void MOTOR_setSpeed(uint dutyCycle, int motor)
{
    if (dutyCycle < 0)
    {
        dutyCycle = 0;
    }
    else if (dutyCycle > 100)
    {
        dutyCycle = 100;
    }

    // Convert duty cycle percentage to PWM level
    uint16_t level = dutyCycle / 100.f * PWM_CYCLE;

    if (motor & MOTOR_LEFT)
    {
        pwm_set_chan_level(pwm_slice, PWM_CHAN_A, level);
        motorSpeedLeft = dutyCycle;
    }

    if (motor & MOTOR_RIGHT)
    {
        pwm_set_chan_level(pwm_slice, PWM_CHAN_B, level);
        motorSpeedRight = dutyCycle;
    }
}

// Get current motor speed (range: 0-100%)
uint MOTOR_getSpeed(int motor)
{
    // Return motor speed in percentage
    if (motor & MOTOR_LEFT)
    {
        return motorSpeedLeft;
    }
    else if (motor & MOTOR_RIGHT)
    {
        return motorSpeedRight;
    }
    else
    {
        return 0;
    }
}

// Set motor direction (forward or reverse)
void MOTOR_setDirection(int dir, int motor)
{
    // Map motor direction to GPIO pin states
    if (dir == MOTOR_DIR_FORWARD)
    {
        if (motor & MOTOR_LEFT)
        {
            gpio_put(GPIO_PIN_MOTOR_IN3, 1);
            gpio_put(GPIO_PIN_MOTOR_IN4, 0);
            motorDirLeft = MOTOR_DIR_FORWARD;
        }

        if (motor & MOTOR_RIGHT)
        {
            gpio_put(GPIO_PIN_MOTOR_IN1, 1);
            gpio_put(GPIO_PIN_MOTOR_IN2, 0);
            motorDirRight = MOTOR_DIR_FORWARD;
        }
    }
    else if (dir == MOTOR_DIR_REVERSE)
    {
        if (motor & MOTOR_LEFT)
        {
            gpio_put(GPIO_PIN_MOTOR_IN3, 0);
            gpio_put(GPIO_PIN_MOTOR_IN4, 1);
            motorDirLeft = MOTOR_DIR_REVERSE;
        }

        if (motor & MOTOR_RIGHT)
        {
            gpio_put(GPIO_PIN_MOTOR_IN1, 0);
            gpio_put(GPIO_PIN_MOTOR_IN2, 1);
            motorDirRight = MOTOR_DIR_REVERSE;
        }
    }
    else
    {
        // Invalid direction
        return;
    }
}

// Get current motor direction (forward or reverse)
int MOTOR_getDirection(int motor)
{
    // Return motor direction (-1: forward, 1: reverse)
    if (motor & MOTOR_LEFT)
    {
        return motorDirLeft;
    }
    else if (motor & MOTOR_RIGHT)
    {
        return motorDirRight;
    }
    else
    {
        return -1;
    }
}

// Stop both motors
void MOTOR_stop(int motor)
{
    MOTOR_setSpeed(0, motor);
}

bool pidStopCallback(struct repeating_timer *timer)
{
    PID_setTargetSpeed(pidLeft, SPEED_NONE);
    PID_setTargetSpeed(pidRight, SPEED_NONE);
    return false;
}

// Motor Turn Functions
void MOTOR_spotTurn(int turnDirection, int angle)
{
    if (angle < MIN_TURN_ANGLE)
    {
        angle = MIN_TURN_ANGLE;
    }

    // Adjust angle to be a multiple of MIN_TURN_ANGLE
    int angleError = angle % MIN_TURN_ANGLE;
    if (angleError > 0)
    {
        if (angleError > MIN_TURN_ANGLE / 2)
        {
            // Round up to next multiple of MIN_TURN_ANGLE
            angle += MIN_TURN_ANGLE - angleError;
        }
        else
        {
            // Round down to previous multiple of MIN_TURN_ANGLE
            angle -= angleError;
        }
    }

    // Calculate number of interrupts needed for the turn
    int interrupts = 4 * angle / MIN_TURN_ANGLE;

    // Set turn direction
    if (turnDirection == MOTOR_TURN_CLOCKWISE)
    {
        MOTOR_setRightTurnMode();
    }
    else
    {
        MOTOR_setLeftTurnMode();
    }

    // Start the motors at the specified speed
    MOTOR_setSpeed(80, MOTOR_LEFT | MOTOR_RIGHT);

    // Wait for the specified number of interrupts
    ENCODER_waitForISRInterrupts(interrupts);

    // Stop the motors after the turn is complete
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
}

// Spot turn function using PID controller
void MOTOR_spotTurnPID(PID *pidLeft, PID *pidRight, int turnDirection, int angle)
{
    if (angle < MIN_TURN_ANGLE)
    {
        angle = MIN_TURN_ANGLE;
    }

    // Adjust angle to be a multiple of MIN_TURN_ANGLE
    int angleError = angle % MIN_TURN_ANGLE;
    if (angleError > 0)
    {
        if (angleError > MIN_TURN_ANGLE / 2)
        {
            // Round up to next multiple of MIN_TURN_ANGLE
            angle += MIN_TURN_ANGLE - angleError;
        }
        else
        {
            // Round down to previous multiple of MIN_TURN_ANGLE
            angle -= angleError;
        }
    }

    // Calculate number of interrupts needed for the turn
    int interrupts = 4 * angle / MIN_TURN_ANGLE;

    // Set turn direction
    if (turnDirection == MOTOR_TURN_CLOCKWISE)
    {
        MOTOR_setRightTurnMode();
    }
    else
    {
        MOTOR_setLeftTurnMode();
    }

    // Set PID target speeds
    PID_setTargetSpeed(pidLeft, SPEED_MEDIUM);
    PID_setTargetSpeed(pidRight, SPEED_MEDIUM);

    // Start the motors using PID control
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when turn is done
}

// Move forward a specified distance (cm)
void MOTOR_moveFoward(int cm)
{
    // Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);

    // Set both motors to forward direction
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    // Set motor speeds
    int speedl = 70;
    int speedR = 68;

    // Start the motors forward until the specified distance is reached
    MOTOR_setSpeed(speedl, MOTOR_LEFT);
    MOTOR_setSpeed(speedR, MOTOR_RIGHT);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
}

// Move backward a specified distance (cm)
void MOTOR_moveBackward(int cm)
{
    // Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);

    // Set both motors to reverse direction
    MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_LEFT | MOTOR_RIGHT);

    // Set motor speed
    int speed = 80;

    // Start the motors backward until the specified distance is reached
    MOTOR_setSpeed(speed, MOTOR_LEFT | MOTOR_RIGHT);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
}

// Move forward a specified distance (cm) using PID control
void MOTOR_moveFowardPID(PID *pidLeft, PID *pidRight, int cm)
{
    int interrupts = ENCODER_cmToSteps(cm);

    // Set both motors to forward direction
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    // Start the motors forward using PID control until the specified distance is reached
    PID_setTargetSpeed(pidLeft, SPEED_HIGH);
    PID_setTargetSpeed(pidRight, SPEED_HIGH);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when movement is done
}

// Move backward a specified distance (cm) using PID control
void MOTOR_moveReversePID(PID *pidLeft, PID *pidRight, int cm)
{
    int interrupts = ENCODER_cmToSteps(cm);

    // Set both motors to reverse direction
    MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_LEFT | MOTOR_RIGHT);

    // Start the motors backward using PID control until the specified distance is reached
    PID_setTargetSpeed(pidLeft, SPEED_HIGH);
    PID_setTargetSpeed(pidRight, SPEED_HIGH);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when movement is done
}