#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define GPIO_PIN_PWM_EN1 14
#define GPIO_PIN_PWM_EN2 15
#define GPIO_PIN_MOTOR_IN1 6
#define GPIO_PIN_MOTOR_IN2 7
#define GPIO_PIN_MOTOR_IN3 8
#define GPIO_PIN_MOTOR_IN4 9


#define PWM_CYCLE 254

static uint pwm_slice;

// Current motor speed
static uint8_t motorSpeedLeft = 0;
static uint8_t motorSpeedRight = 0;

#define MOTOR_LEFT 1
#define MOTOR_RIGHT 2

#define MOTOR_DIR_FORWARD 0
#define MOTOR_DIR_REVERSE 1

void initializeMotor() {
    gpio_init(GPIO_PIN_MOTOR_IN1);
    gpio_init(GPIO_PIN_MOTOR_IN2);
    gpio_init(GPIO_PIN_MOTOR_IN3);
    gpio_init(GPIO_PIN_MOTOR_IN4);
    gpio_set_dir(GPIO_PIN_MOTOR_IN1, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN2, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN3, GPIO_OUT);
    gpio_set_dir(GPIO_PIN_MOTOR_IN4, GPIO_OUT);

    gpio_set_function(GPIO_PIN_PWM_EN1, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_PIN_PWM_EN2, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(GPIO_PIN_PWM_EN1);

    pwm_config c = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&c, 5);
    pwm_config_set_wrap(&c, PWM_CYCLE - 1);

    // Set default state of motor
    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);
    
    pwm_init(pwm_slice, &c, true);
}

void moveMotorForward() {
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, PWM_CYCLE); // Set maximum duty cycle for full speed
    pwm_set_chan_level(pwm_slice, PWM_CHAN_B, PWM_CYCLE); // Set maximum duty cycle for full speed
    gpio_put(GPIO_PIN_MOTOR_IN1, 1);
    gpio_put(GPIO_PIN_MOTOR_IN2, 0);
    gpio_put(GPIO_PIN_MOTOR_IN3, 1);
    gpio_put(GPIO_PIN_MOTOR_IN4, 0);
}

void moveMotorBackward() {
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, PWM_CYCLE); // Set maximum duty cycle for full speed
    pwm_set_chan_level(pwm_slice, PWM_CHAN_B, PWM_CYCLE); // Set maximum duty cycle for full speed
    gpio_put(GPIO_PIN_MOTOR_IN1, 0);
    gpio_put(GPIO_PIN_MOTOR_IN2, 1);
    gpio_put(GPIO_PIN_MOTOR_IN3, 0);
    gpio_put(GPIO_PIN_MOTOR_IN4, 1);
}

int main() {
    stdio_init_all();
    initializeMotor();
    moveMotorForward();
    sleep_ms(20000);  // Move forward for 5 seconds
    pwm_set_chan_level(pwm_slice, PWM_CHAN_A, 0); // Stop motor
    return 0;
}
