#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define GPIO_PIN_MOTOR_IN1 2
#define GPIO_PIN_MOTOR_IN2 3
#define GPIO_PIN_PWM_EN1 4

#define PWM_CYCLE 254

static uint pwm_slice;

void initializeMotor()
{
    gpio_init(GPIO_PIN_MOTOR_IN1);
    gpio_init(GPIO_PIN_MOTOR_IN2);
    gpio_set_dir(GPIO_PIN_MOTOR_IN1, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN2, GPIO_OUT);

    gpio_set_function(GPIO_PIN_PWM_EN1, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN1);

    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&c, 5);
    pwm_config_set_wrap(&c, PWM_CYCLE - 1);

    pwm_init(pwm_slice, &c, true);
}

void moveMotor1Forward()
{
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, PWM_CYCLE); // Set maximum duty cycle for full speed
    gpio_put(GPIO_PIN_MOTOR_IN1, 1);
    gpio_put(GPIO_PIN_MOTOR_IN2, 0);
}

int main()
{
    stdio_init_all();
    initializeMotor();
    moveMotor1Forward();
    sleep_ms(20000);                              // Move forward for 5 seconds
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, 0); // Stop motor
    return 0;
}