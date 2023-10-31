#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include "motor.h"
#include "pid.h"
#include "../encoder/encoder.h"

/*
====== Motor Control ======
IN1 IN2, EN1 -> Right Motor
IN3 IN4, EN2 -> Left Motor
====== ============= ======
*/

#define GPIO_PIN_PWM_EN1 14
#define GPIO_PIN_PWM_EN2 15
#define GPIO_PIN_MOTOR_IN1 6
#define GPIO_PIN_MOTOR_IN2 7
#define GPIO_PIN_MOTOR_IN3 8
#define GPIO_PIN_MOTOR_IN4 9

#define PWM_CYCLE 254

#define MIN_TURN_ANGLE 45

static uint pwm_slice;

// Current motor speed
static uint8_t motorSpeedLeft = 0;
static uint8_t motorSpeedRight = 0;

// Current motor direction
static int motorDirLeft = -1;
static int motorDirRight = -1;

static PID *pidLeft;
static PID *pidRight;

void MOTOR_init(PID *left, PID *right)
{
    printf("[Motor] Init start \n");

    // Initialize GPIO pins
    gpio_init(GPIO_PIN_MOTOR_IN1);
    gpio_init(GPIO_PIN_MOTOR_IN2);
    gpio_init(GPIO_PIN_MOTOR_IN3);
    gpio_init(GPIO_PIN_MOTOR_IN4);
    gpio_set_dir(GPIO_PIN_MOTOR_IN1, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN2, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN3, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN4, GPIO_OUT);

    // Initialize PWM
    gpio_set_function(GPIO_PIN_PWM_EN1, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_PIN_PWM_EN2, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN1);
    // assert(pwm_slice == pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN2));
    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&c, 5);       // clk_sys defaults at 125 MHz, PWM freq is (clk_sys / div)
    pwm_config_set_wrap(&c, PWM_CYCLE - 1); // Set period of PWM_CYCLE cycles (0 to PWM_CYCLE-1)

    // Set default state of motor
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    pwm_init(pwm_slice, &c, true);

    pidLeft = left;
    pidRight = right;

    printf("[Motor] Init done \n");
}

//Motor speed setter and getter functions
void MOTOR_setSpeed(uint dutyCycle, int motor)
{
    if (dutyCycle < 0)
        dutyCycle = 0;
    else if (dutyCycle > 100)
        dutyCycle = 100;
    // Convert to its appropriate PWM level from percentage
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

uint MOTOR_getSpeed(int motor)
{
    // Return motor duty cycle in percentage
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

//Motor direction setter and getter functions
void MOTOR_setDirection(int dir, int motor)
{
    // Motor 1: IN1 IN2
    // 10 = Forward, 01 = Reverse
    // Motor 2: IN3 IN4
    // 01 = Forward, 10 = Reverse
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
}

int MOTOR_getDirection(int motor)
{
    if (motor == MOTOR_LEFT)
    {
        return motorDirLeft;
    }
    else if (motor == MOTOR_RIGHT)
    {
        return motorDirRight;
    }
    return -1;
}

void MOTOR_setLeftTurnMode(void)
{
    MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_LEFT);
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_RIGHT);
}

void MOTOR_setRightTurnMode(void)
{
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT);
    MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_RIGHT);
}

//Motor Stop Functions
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

    //Motor will turn based on the number of interrupt count
    int interrupts = 4 * angle / MIN_TURN_ANGLE;
    int speed = 80;

    //Motor will turn right/left based on direction set
    if (turnDirection == MOTOR_TURN_CLOCKWISE)
        MOTOR_setRightTurnMode();
    else if (turnDirection == MOTOR_TURN_ANTICLOCKWISE)
        MOTOR_setLeftTurnMode();

    //Turn until interrupt count is reached
    MOTOR_setSpeed(speed, MOTOR_LEFT | MOTOR_RIGHT);
    ENCODER_waitForISRInterrupts(interrupts);
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
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

    //Motor will turn based on the number of interrupt count
    int interrupts = 4 * angle / MIN_TURN_ANGLE;

    //Motor will turn right/left based on direction set
    if (turnDirection == MOTOR_TURN_CLOCKWISE)
        MOTOR_setRightTurnMode();
    else if (turnDirection == MOTOR_TURN_ANTICLOCKWISE)
        MOTOR_setLeftTurnMode();

    //Turn until interrupt count is reached
    PID_setTargetSpeed(pidLeft, SPEED_MEDIUM);
    PID_setTargetSpeed(pidRight, SPEED_MEDIUM);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when turn is done
}

// Move Specific Distance Functions
void MOTOR_moveFoward(int cm)
{
    //Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);
    int speed = 80;

    // Set Motor Direction Foward
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    // Go forward until interrupt count is reached
    MOTOR_setSpeed(speed, MOTOR_LEFT | MOTOR_RIGHT);
    ENCODER_waitForISRInterrupts(interrupts); 
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT); 
}

void MOTOR_moveBackward(int cm)
{
    //Convert distance(in cm) to interrupt count
    int interrupts = ENCODER_cmToSteps(cm);
    int speed = 80;

    // Set Motor Direction Backwards
    MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_LEFT | MOTOR_RIGHT);

    // Go Backwards until interrupt count is reached
    MOTOR_setSpeed(speed, MOTOR_LEFT | MOTOR_RIGHT);
    ENCODER_waitForISRInterrupts(interrupts); 
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);  
}

void MOTOR_moveFowardPID(PID *pidLeft, PID *pidRight, int cm)
{
    int interrupts = ENCODER_cmToSteps(cm);

    // Set Motor Direction Foward
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);

    // Go forward until interrupt count is reached
    PID_setTargetSpeed(pidLeft, SPEED_MEDIUM);
    PID_setTargetSpeed(pidRight, SPEED_MEDIUM);
    ENCODER_alertAfterISRInterrupts(interrupts, pidStopCallback); // Setup timer to alert when movement is done
}