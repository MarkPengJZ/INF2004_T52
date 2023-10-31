#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "motor.h"
#include "pid.h"
#include "../encoder/encoder.h"

/*
-- Motor Management ======
IN1 IN2, EN1 -> Right Motor
IN3 IN4, EN2 -> Left Motor
-- Motor Configuration --
*/

#define RIGHT_MOTOR_PWM_PIN 14
#define LEFT_MOTOR_PWM_PIN 15
#define RIGHT_MOTOR_PIN_1 6
#define RIGHT_MOTOR_PIN_2 7
#define LEFT_MOTOR_PIN_1 8
#define LEFT_MOTOR_PIN_2 9

#define PWM_CYCLE 254

#define MIN_TURN_ANGLE 45

static uint pwm_slice;

// Current motor speed
static uint8_t leftMotorSpeed = 0;
static uint8_t rightMotorSpeed = 0;

// Current motor direction
static int leftMotorDirection = -1;
static int rightMotorDirection = -1;

static PID *pidLeft;
static PID *pidRight;

void initializeMotor(PID *left, PID *right)
{
    printf("[Motor] Init start \n");

    // Initialize GPIO pins
    gpio_init(RIGHT_MOTOR_PIN_1);
    gpio_init(RIGHT_MOTOR_PIN_2);
    gpio_init(LEFT_MOTOR_PIN_1);
    gpio_init(LEFT_MOTOR_PIN_2);
    gpio_set_dir(RIGHT_MOTOR_PIN_1, GPIO_OUT);
    gpio_set_dir(RIGHT_MOTOR_PIN_2, GPIO_OUT);
    gpio_set_dir(LEFT_MOTOR_PIN_1, GPIO_OUT);
    gpio_set_dir(LEFT_MOTOR_PIN_2, GPIO_OUT);

    // Initialize PWM
    gpio_set_function(RIGHT_MOTOR_PWM_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LEFT_MOTOR_PWM_PIN, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(RIGHT_MOTOR_PWM_PIN);
    // assert(pwm_slice == pwm_gpio_to_slice_num(LEFT_MOTOR_PWM_PIN));
    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&c, 5);       // clk_sys defaults at 125 MHz, PWM freq is (clk_sys / div)
    pwm_config_set_wrap(&c, PWM_CYCLE - 1); // Set period of PWM_CYCLE cycles (0 to PWM_CYCLE-1)

    // Set default state of motor
    MOTOR_stop(LEFT_MOTOR | RIGHT_MOTOR);
    setMotorDirection(DIRECTION_FORWARD, LEFT_MOTOR | RIGHT_MOTOR);

    pwm_init(pwm_slice, &c, true);

    pidLeft = left;
    pidRight = right;

    printf("[Motor] Init done \n");
}

// Motor speed setter and getter functions
void setMotorSpeed(uint dutyCycle, int motor)
{
    if (dutyCycle < 0)
        dutyCycle = 0;
    else if (dutyCycle > 100)
        dutyCycle = 100;
    // Convert to its appropriate PWM level from percentage
    uint16_t level = dutyCycle / 100.f * PWM_CYCLE;
    if (motor & LEFT_MOTOR)
    {
        pwm_set_chan_level(pwm_slice, PWM_CHAN_A, level);
        leftMotorSpeed = dutyCycle;
    }
    if (motor & RIGHT_MOTOR)
    {
        pwm_set_chan_level(pwm_slice, PWM_CHAN_B, level);
        rightMotorSpeed = dutyCycle;
    }
}

uint MOTOR_getSpeed(int motor)
{
    // Return motor duty cycle in percentage
    if (motor & LEFT_MOTOR)
    {
        return leftMotorSpeed;
    }
    else if (motor & RIGHT_MOTOR)
    {
        return rightMotorSpeed;
    }
    else
    {
        return 0;
    }
}

// Motor direction setter and getter functions
void setMotorDirection(int dir, int motor)
{
    // Motor 1: IN1 IN2
    // 10 = Forward, 01 = Reverse
    // Motor 2: IN3 IN4
    // 01 = Forward, 10 = Reverse
    if (dir == DIRECTION_FORWARD)
    {
        if (motor & LEFT_MOTOR)
        {
            gpio_put(LEFT_MOTOR_PIN_1, 1);
            gpio_put(LEFT_MOTOR_PIN_2, 0);
            leftMotorDirection = DIRECTION_FORWARD;
        }
        if (motor & RIGHT_MOTOR)
        {
            gpio_put(RIGHT_MOTOR_PIN_1, 1);
            gpio_put(RIGHT_MOTOR_PIN_2, 0);
            rightMotorDirection = DIRECTION_FORWARD;
        }
    }
    else if (dir == DIRECTION_REVERSE)
    {
        if (motor & LEFT_MOTOR)
        {
            gpio_put(LEFT_MOTOR_PIN_1, 0);
            gpio_put(LEFT_MOTOR_PIN_2, 1);
            leftMotorDirection = DIRECTION_REVERSE;
        }
        if (motor & RIGHT_MOTOR)
        {
            gpio_put(RIGHT_MOTOR_PIN_1, 0);
            gpio_put(RIGHT_MOTOR_PIN_2, 1);
            rightMotorDirection = DIRECTION_REVERSE;
        }
    }
}

int MOTOR_getDirection(int motor)
{
    if (motor == LEFT_MOTOR)
    {
        return leftMotorDirection;
    }
    else if (motor == RIGHT_MOTOR)
    {
        return rightMotorDirection;
    }
    return -1;
}

void MOTOR_setLeftTurnMode(void)
{
    setMotorDirection(DIRECTION_REVERSE, LEFT_MOTOR);
    setMotorDirection(DIRECTION_FORWARD, RIGHT_MOTOR);
}

void MOTOR_setRightTurnMode(void)
{
    setMotorDirection(DIRECTION_FORWARD, LEFT_MOTOR);
    setMotorDirection(DIRECTION_REVERSE, RIGHT_MOTOR);
}

// Motor Stop Functions
void MOTOR_stop(int motor)
{
    setMotorSpeed(0, motor);
}

bool pidStopCallback(struct repeating_timer *timer)
{
    PID_setTargetSpeed(pidLeft, NO_SPEED);
    PID_setTargetSpeed(pidRight, NO_SPEED);
    return false;
}

// Motor Turn Functions
void MOTOR_spotTurn(int turnDirection, int angle)
{
    if (angle < MIN_TURN_ANGLE)
        angle = MIN_TURN_ANGLE;

    // Ensure angle is a multiple of MIN_TURN_ANGLE
    int angleError = angle % MIN_TURN_ANGLE;
    if (angleError > 0)
    {
        if (angleError > MIN_TURN_ANGLE / 2)
        {
            // Closer to next multiple of MIN_TURN_ANGLE, round up
            angle += MIN_TURN_ANGLE - angleError;
        }
        else
        {
            // Closer to last multiple of MIN_TURN_ANGLE, round down
            angle -= angleError;
        }
    }

    // Motor will turn based on the number of interrupt count
    int interrupts = 4 * angle / MIN_TURN_ANGLE;
    int speed = 80;

    // Motor will turn right/left based on direction set
    if (turnDirection == TURN_CLOCKWISE)
        MOTOR_setRightTurnMode();
    else if (turnDirection == TURN_COUNTERCLOCKWISE)
        MOTOR_setLeftTurnMode();

    // Turn until interrupt count is reached
    setMotorSpeed(speed, LEFT_MOTOR | RIGHT_MOTOR);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(LEFT_MOTOR | RIGHT_MOTOR);
}

void MOTOR_spotTurnPID(PID *pidLeft, PID *pidRight, int turnDirection, int angle)
{
    if (angle < MIN_TURN_ANGLE)
        angle = MIN_TURN_ANGLE;

    // Ensure angle is a multiple of MIN_TURN_ANGLE
    int angleError = angle % MIN_TURN_ANGLE;
    if (angleError > 0)
    {
        if (angleError > MIN_TURN_ANGLE / 2)
        {
            // Closer to next multiple of MIN_TURN_ANGLE, round up
            angle += MIN_TURN_ANGLE - angleError;
        }
        else
        {
            // Closer to last multiple of MIN_TURN_ANGLE, round down
            angle -= angleError;
        }
    }

    // Motor will turn based on the number of interrupt count
    int interrupts = 4 * angle / MIN_TURN_ANGLE;

    // Motor will turn right/left based on direction set
    if (turnDirection == TURN_CLOCKWISE)
        MOTOR_setRightTurnMode();
    else if (turnDirection == TURN_COUNTERCLOCKWISE)
        MOTOR_setLeftTurnMode();

    // Turn until interrupt count is reached
    PID_setTargetSpeed(pidLeft, MEDIUM_SPEED);
    PID_setTargetSpeed(pidRight, MEDIUM_SPEED);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when turn is done
}

// Move Specific Distance Functions
void MOTOR_moveFoward(int cm)
{
    // Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);
    int speed = 80;

    // Set Motor Direction Foward
    setMotorDirection(DIRECTION_FORWARD, LEFT_MOTOR | RIGHT_MOTOR);

    // Go forward until interrupt count is reached
    setMotorSpeed(speed, LEFT_MOTOR | RIGHT_MOTOR);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(LEFT_MOTOR | RIGHT_MOTOR);
}

void MOTOR_moveBackward(int cm)
{
    // Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);
    int speed = 80;

    // Set Motor Direction Backwards
    setMotorDirection(DIRECTION_REVERSE, LEFT_MOTOR | RIGHT_MOTOR);

    // Go Backwards until interrupt count is reached
    setMotorSpeed(speed, LEFT_MOTOR | RIGHT_MOTOR);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(LEFT_MOTOR | RIGHT_MOTOR);
}

void MOTOR_moveFowardPID(PID *pidLeft, PID *pidRight, int cm)
{
    int interrupts = ENCODER_cmToSteps(cm);

    // Set Motor Direction Foward
    setMotorDirection(DIRECTION_FORWARD, LEFT_MOTOR | RIGHT_MOTOR);

    // Go forward until interrupt count is reached
    PID_setTargetSpeed(pidLeft, MEDIUM_SPEED);
    PID_setTargetSpeed(pidRight, MEDIUM_SPEED);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when movement is done
}