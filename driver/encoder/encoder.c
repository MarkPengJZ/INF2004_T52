#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"

#include "encoder.h"

// Encoder pin definitions
#define GPIO_PIN_ENC_LEFT 2
#define GPIO_PIN_ENC_RIGHT 3

// Define buffer size for storing encoder interrupt counts
#define INTERRUPT_BUF_SIZE 5

// Encoder disc parameters
#define ENCODER_DISC_SLOTS 20
#define WHEEL_DIAMETER_CM 6.7 // Wheel diameter in centimeters

// Calculate wheel circumference
const float WHEEL_CIRCUMFERENCE_CM = M_PI * WHEEL_DIAMETER_CM;

// Distance per encoder slot
const float CM_PER_SLOT = WHEEL_CIRCUMFERENCE_CM / ENCODER_DISC_SLOTS;

// Timer for wheel speed calculation
struct repeating_timer timer;

// Timer for precise distance movement and angle spot turns
struct repeating_timer isrAlertTimer;

// Wheel speed calculation variables
volatile int encIndex = 0;                             // Index for interrupt buffer
volatile int leftInterruptBuffer[INTERRUPT_BUF_SIZE];  // Interrupt buffer for left encoder
volatile int rightInterruptBuffer[INTERRUPT_BUF_SIZE]; // Interrupt buffer for right encoder

// Encoder interrupt counting variables
volatile int enableISRCounter = 0;             // Flag to enable/disable ISR counter
volatile int encoderISRLeftTargetReached = 0;  // Flag indicating left encoder has reached target count
volatile int encoderISRRightTargetReached = 0; // Flag indicating right encoder has reached target count
volatile uint encoderISRTarget = 0;            // Target interrupt count
volatile uint encoderISRLeftCounter = 0;       // Current interrupt count for left encoder
volatile uint encoderISRRightCounter = 0;      // Current interrupt count for right encoder

// Encoder interrupt handler
void ISR_encoder(uint gpio, uint32_t events)
{
    if (gpio == GPIO_PIN_ENC_LEFT)
    {
        leftInterruptBuffer[encIndex]++; // Increment left encoder interrupt count

        if (enableISRCounter && !encoderISRLeftTargetReached)
        {                            // Check if ISR counter is enabled and left encoder target not reached
            encoderISRLeftCounter++; // Increment left encoder ISR counter

            if (encoderISRLeftCounter == encoderISRTarget)
            {                                    // Check if left encoder ISR counter has reached target
                encoderISRLeftTargetReached = 1; // Set flag indicating left encoder target reached
            }
        }
    }

    if (gpio == GPIO_PIN_ENC_RIGHT)
    {
        rightInterruptBuffer[encIndex]++; // Increment right encoder interrupt count

        if (enableISRCounter && !encoderISRRightTargetReached)
        {                             // Check if ISR counter is enabled and right encoder target not reached
            encoderISRRightCounter++; // Increment right encoder ISR counter

            if (encoderISRRightCounter == encoderISRTarget)
            {                                     // Check if right encoder ISR counter has reached target
                encoderISRRightTargetReached = 1; // Set flag indicating right encoder target reached
            }
        }
    }
}

// Timer callback for wheel speed calculation
bool timerCallback(struct repeating_timer *timer)
{
    // Update index for interrupt buffer
    encIndex++;
    if (encIndex == INTERRUPT_BUF_SIZE)
    {
        encIndex = 0;
    }

    // Reset interrupt buffers for new calculations
    leftInterruptBuffer[encIndex] = 0;
    rightInterruptBuffer[encIndex] = 0;

    // Return true to continue timer
    return true;
}

// Initialize encoder hardware and timer for wheel speed calculation
void ENCODER_init(void)
{
    printf("[Encoder] Init start \n");

    // Set up timer to trigger wheel speed calculation callback every 0.25 seconds
    add_repeating_timer_ms(-250, &timerCallback, NULL, &timer);

    // Set up encoder interrupts to trigger on falling edge (high to low transition)
    gpio_set_irq_enabled_with_callback(GPIO_PIN_ENC_LEFT, GPIO_IRQ_EDGE_FALL, true, &ISR_encoder);
    gpio_set_irq_enabled_with_callback(GPIO_PIN_ENC_RIGHT, GPIO_IRQ_EDGE_FALL, true, &ISR_encoder);

    printf("[Encoder] Init done \n");
}

// Get wheel speed in centimeters per second
float ENCODER_getWheelSpeed(int encoder)
{
    // Calculate total interrupts for specified encoder
    int totalInterrupts = 0;
    if (encoder & ENCODER_LEFT)
    {
        for (int i = 0; i < INTERRUPT_BUF_SIZE; i++)
        {
            if (i == encIndex)
            {
                continue;
            }
            totalInterrupts += leftInterruptBuffer[i];
        }
    }
    else if (encoder & ENCODER_RIGHT)
    {
        for (int i = 0; i < INTERRUPT_BUF_SIZE; i++)
        {
            if (i == encIndex)
            {
                continue;
            }
            totalInterrupts += rightInterruptBuffer[i];
        }
    }

    // Convert interrupts to wheel speed in centimeters per second
    float wheelSpeed = totalInterrupts * CM_PER_SLOT;

    return wheelSpeed;
}