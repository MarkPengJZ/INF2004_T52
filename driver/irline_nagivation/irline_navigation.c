#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define LEFT_SENSOR_PIN 26
#define N_SAMPLES 10
uint16_t sample_buf[N_SAMPLES];
#define MOVING_AVERAGE_WINDOW 5 // Adjust this to your desired window size
uint16_t sensor_readings[MOVING_AVERAGE_WINDOW];
uint16_t moving_average = 0;

void printhelp()
{
    puts("\nCommands:");
    puts("c0, ...\t: Select ADC channel n");
    puts("s\t: Sample once");
    puts("S\t: Sample many");
    puts("w\t: Wiggle pins");
}

void __not_in_flash_func(adc_capture)(uint16_t *buf, size_t count)
{
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);
    for (int i = 0; i < count; i = i + 1)
        buf[i] = adc_fifo_get_blocking();
    adc_run(false);
    adc_fifo_drain();
}

void updateMovingAverage(uint16_t new_reading)
{
    // Update the circular buffer of sensor readings
    for (int i = MOVING_AVERAGE_WINDOW - 1; i > 0; i--)
    {
        sensor_readings[i] = sensor_readings[i - 1];
    }
    sensor_readings[0] = new_reading;

    // Recalculate the moving average
    moving_average = 0;
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++)
    {
        moving_average += sensor_readings[i];
    }
    moving_average /= MOVING_AVERAGE_WINDOW;
}

int main(void)
{
    stdio_init_all();
    adc_init();

    // Set all pins to input (as far as SIO is concerned)
    gpio_set_dir_all_bits(0);
    gpio_set_function(LEFT_SENSOR_PIN, GPIO_FUNC_SIO);
    gpio_disable_pulls(LEFT_SENSOR_PIN);
    gpio_set_input_enabled(LEFT_SENSOR_PIN, true); // change to false if not working

    while (1)
    {
        char c = getchar();
        printf("%c", c);
        switch (c)
        {
        case 's':
        {
            uint32_t result = adc_read();
            const float conversion_factor = 3.3f / (1 << 12);
            printf("\n0x%03x -> %f V\n", result, result * conversion_factor);
            break;
        }
        case 'S':
        {
            printf("\nStarting capture\n");
            adc_capture(sample_buf, N_SAMPLES);
            printf("Done\n");
            for (int i = 0; i < N_SAMPLES; i = i + 1)
                printf("%d\n", sample_buf[i]);
            break;
        }
        case 'w':
        {
            printf("\nStarting capture: Case W\n");
            while (1)
            {
                uint16_t adc_value = adc_read();
                // printf("\nIR Sensor Value: %d\n", adc_value);
                updateMovingAverage(adc_value);
                printf("\nIR Sensor Value: %d (Moving Average: %d)\n", adc_value, moving_average);

                if (moving_average < 400)
                {
                    printf("\nLine not detected: %d\n", moving_average);
                }
                else
                {
                    printf("\nLine detected: %d\n", moving_average);
                    // break;
                }
                sleep_ms(500);
            }
            break;
        }
        // break;
        // case '\n':
        // case '\r':
        //     break;
        // case 'h':
        //     printhelp();
        //     break;
        default:
            printf("\nUnrecognised command: %c\n", c);
            printhelp();
            break;
        }
    }
}