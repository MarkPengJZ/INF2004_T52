#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"

#include "encoder.h"

#define GPIO_PIN_ENC_LEFT 2
#define GPIO_PIN_ENC_RIGHT 3

#define INTERRUPT_BUF_SIZE 5

#define ENCODER_DISC_SLOTS 20
#define WHEEL_DIAMETER_CM 6.7                             // Centimeter
#define WHEEL_CIRCUMFERENCE_CM (M_PI * WHEEL_DIAMETER_CM) // Centimeter

const float CM_PER_SLOT = WHEEL_CIRCUMFERENCE_CM / ENCODER_DISC_SLOTS;

struct repeating_timer timer, isrAlertTimer;

// Wheel speed calculation
volatile int encIndex = 0;
volatile int leftInterruptBuffer[INTERRUPT_BUF_SIZE];
volatile int rightInterruptBuffer[INTERRUPT_BUF_SIZE];

// Encoder interrupt counting (used during precise distance movement and angle spot turns)
volatile int enableISRCounter = 0;
volatile int encoderISRLeftTargetReached = 0;
volatile int encoderISRRightTargetReached = 0;
volatile uint encoderISRTarget = 0;
volatile uint encoderISRLeftCounter = 0;
volatile uint encoderISRRightCounter = 0;

void ISR_encoder(uint gpio, uint32_t events)
{
    if (gpio == GPIO_PIN_ENC_LEFT)
    {
        leftInterruptBuffer[encIndex]++;
        if (enableISRCounter && !encoderISRLeftTargetReached)
        {
            encoderISRLeftCounter++;

            if (encoderISRLeftCounter == encoderISRTarget)
                encoderISRLeftTargetReached = 1;
        }
        // printf("L encoder interrupt \n");
    }

    if (gpio == GPIO_PIN_ENC_RIGHT)
    {
        rightInterruptBuffer[encIndex]++;
        if (enableISRCounter && !encoderISRRightTargetReached)
        {
            encoderISRRightCounter++;

            if (encoderISRRightCounter == encoderISRTarget)
                encoderISRRightTargetReached = 1;
        }
        // printf("R encoder interrupt \n");
    }
}

bool timerCallback(struct repeating_timer *timer)
{
    /*
    ===== How the speed calculation works =====
    1. Both encoder has its own int[4] buffer (+1 for the current buffer, which may not be complete result yet)
    2. A timer is setup to trigger this callback every 0.25s
    3. Stores the amount of interrupts since the last callback
    4. The buffer is filled sequentially with index incremented each time, wrapping back to 0 at max index
    5. The speed is calculated by summing up values in the buffer (average interrupts/second for the last second)
    */
    encIndex++;
    if (encIndex == INTERRUPT_BUF_SIZE)
        encIndex = 0;
    leftInterruptBuffer[encIndex] = 0;
    rightInterruptBuffer[encIndex] = 0;
    return true;
}

void ENCODER_init(void)
{
    printf("[Encoder] Init start \n");

    // Setup timer (0.25 second), used to calculate wheel speed
    // Create a repeating timer that calls repeating_timer_callback.
    // If the delay is > 0 then this is the delay between the previous callback ending and the next starting.
    // If the delay is negative then the next call to the callback will be exactly delay ms after the start of the call to the last callback
    add_repeating_timer_ms(-250, &timerCallback, NULL, &timer);

    // Setup interrupt, trigger during high to low transition
    gpio_set_irq_enabled_with_callback(GPIO_PIN_ENC_LEFT, GPIO_IRQ_EDGE_FALL, true, &ISR_encoder);
    gpio_set_irq_enabled_with_callback(GPIO_PIN_ENC_RIGHT, GPIO_IRQ_EDGE_FALL, true, &ISR_encoder);

    printf("[Encoder] Init done \n");
}

// Returns wheel speed in centimeter/sec
float ENCODER_getWheelSpeed(int encoder)
{
    int interruptsPerSec = ENCODER_getWheelInterruptSpeed(encoder);
    return interruptsPerSec * CM_PER_SLOT;
}

// Returns wheel speed in interrupts/sec
int ENCODER_getWheelInterruptSpeed(int encoder)
{
    int total = 0;
    if (encoder & ENCODER_LEFT)
    {
        for (int i = 0; i < INTERRUPT_BUF_SIZE; i++)
        {
            if (i == encIndex)
                continue;
            total += leftInterruptBuffer[i];
        }
    }
    else if (encoder & ENCODER_RIGHT)
    {
        for (int i = 0; i < INTERRUPT_BUF_SIZE; i++)
        {
            if (i == encIndex)
                continue;
            total += rightInterruptBuffer[i];
        }
    }
    return total;
}

void ENCODER_waitForISRInterrupts(uint target)
{
    // Reset and enable ISR counter
    encoderISRLeftCounter = encoderISRRightCounter = 0;
    encoderISRLeftTargetReached = encoderISRRightTargetReached = 0;
    encoderISRTarget = target;
    enableISRCounter = 1;

    // Block until interrupt count reaches target
    while (!encoderISRLeftTargetReached || !encoderISRRightTargetReached)
        sleep_ms(10);

    enableISRCounter = 0;
}

bool timerCallbackISRAlert(struct repeating_timer *timer)
{
    if (enableISRCounter)
    {
        if (encoderISRLeftTargetReached && encoderISRRightTargetReached)
        {
            repeating_timer_callback_t callback = (repeating_timer_callback_t)(timer->user_data);
            enableISRCounter = 0;
            return callback(timer); // Stops the timer
        }
        else
        {
            return true;
        }
    }
}

void ENCODER_alertAfterISRInterrupts(uint target, repeating_timer_callback_t callback)
{
    // Reset and enable ISR counter
    encoderISRLeftCounter = encoderISRRightCounter = 0;
    encoderISRLeftTargetReached = encoderISRRightTargetReached = 0;
    encoderISRTarget = target;
    enableISRCounter = 1;

    add_repeating_timer_ms(-10, timerCallbackISRAlert, callback, &isrAlertTimer);
}

// Convert from centimeters to steps
int ENCODER_cmToSteps(float cm)
{
    return (int)(cm / CM_PER_SLOT); // Convert to an integer (note this is NOT rounded)
}