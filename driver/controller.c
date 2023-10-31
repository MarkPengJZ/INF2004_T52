#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include "motor/motor.h"
#include "motor/pid.h"
#include "encoder/encoder.h"

#define PID_Kp 2.f
#define PID_Ki 2.f
#define PID_Kd 0.f

typedef struct state
{
    absolute_time_t currentTime, previousTime, bootTime;

    float deltaTime; // Time since last loop in seconds
    float leftWheelSpeed, rightWheelSpeed;
    uint leftMotorDutyCycle, rightMotorDutyCycle;

    PID *leftPID, *rightPID;    // PID controller data
    uint8_t usePid;             // Flag, PID enabled or disabled
    repeating_timer_t pidTimer; // Repeating timer that runs PID controller
} State;

void togglePid(State *state)
{
    state->usePid ^= 1;
    printf("> [PID] Enabled: %u \n", state->usePid);
}

bool pidCallback(repeating_timer_t *timer)
{
    State state = *((State *)timer->user_data);

    if (state.usePid)
    {
        state.leftMotorDutyCycle = PID_run(state.leftPID, state.leftWheelSpeed, state.deltaTime);
        state.rightMotorDutyCycle = PID_run(state.rightPID, state.rightWheelSpeed, state.deltaTime);

        if (state.leftMotorDutyCycle != MOTOR_getSpeed(MOTOR_LEFT))
        {
            MOTOR_setSpeed(state.leftMotorDutyCycle, MOTOR_LEFT);
        }
        if (state.rightMotorDutyCycle != MOTOR_getSpeed(MOTOR_RIGHT))
        {
            MOTOR_setSpeed(state.rightMotorDutyCycle, MOTOR_RIGHT);
        }

        printf("T: %.2fs | SP: %.2f | SPD L: %.2f | DUTY L: %u | [P]%.2f [I]%.2f [D]%.2f (Err: %.2f) \n", absolute_time_diff_us(state.bootTime, state.currentTime) / 1000000.f, state.leftPID->setPoint, state.leftWheelSpeed, state.leftMotorDutyCycle, state.leftPID->p, state.leftPID->i, state.leftPID->d, state.leftPID->lastError);
        printf("T: %.2fs | SP: %.2f | SPD R: %.2f | DUTY R: %u | [P]%.2f [I]%.2f [D]%.2f (Err: %.2f) \n", absolute_time_diff_us(state.bootTime, state.currentTime) / 1000000.f, state.rightPID->setPoint, state.rightWheelSpeed, state.rightMotorDutyCycle, state.rightPID->p, state.rightPID->i, state.rightPID->d, state.rightPID->lastError);
    }
    else
    {
        printf("SPD L: %.2f | DUTY L: %i \n", state.leftWheelSpeed, MOTOR_getSpeed(MOTOR_LEFT));
        printf("SPD R: %.2f | DUTY R: %i \n", state.rightWheelSpeed, MOTOR_getSpeed(MOTOR_RIGHT));
    }

    return true;
}

int main()
{
    stdio_init_all();
    while (!stdio_usb_connected())
        sleep_ms(100); // Wait for USB to initialize

    printf("Starting PICO!\n");

    State state;

    // Create PID contoller for motors
    state.leftPID = PID_create(PID_Kp, PID_Ki, PID_Kd, 0, 0.f, 100.f);
    state.rightPID = PID_create(PID_Kp, PID_Ki, PID_Kd, 0, 0.f, 100.f);

    // Initialize modules
    MOTOR_init(state.leftPID, state.rightPID);
    ENCODER_init();

    state.usePid = 1;

    while (1)
    {
        uint8_t c = getchar_timeout_us(0);
        switch (c)
        {
        case 'z':
            togglePid(&state);
            break;
        case 'q': // Stop
            if (state.usePid)
            {
                PID_setTargetSpeed(state.leftPID, SPEED_NONE);
                PID_setTargetSpeed(state.rightPID, SPEED_NONE);
                printf("> [Motor] PID Target Setpoint %.0f \n", state.leftPID->setPoint);
            }
            else
            {
                MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
            }
            break;
        case '1': // 60% Duty cycle
            if (state.usePid)
                togglePid(&state);
            MOTOR_setSpeed(60, MOTOR_LEFT | MOTOR_RIGHT);
            break;
        case '2': // 70% Duty cycle
            if (state.usePid)
                togglePid(&state);
            MOTOR_setSpeed(70, MOTOR_LEFT | MOTOR_RIGHT);
            break;
        case '3': // 80% Duty cycle
            if (state.usePid)
                togglePid(&state);
            MOTOR_setSpeed(80, MOTOR_LEFT | MOTOR_RIGHT);
            break;
        case '4': // 90% Duty cycle
            if (state.usePid)
                togglePid(&state);
            MOTOR_setSpeed(90, MOTOR_LEFT | MOTOR_RIGHT);
            break;
        case '5': // 100% Duty cycle
            if (state.usePid)
                togglePid(&state);
            MOTOR_setSpeed(100, MOTOR_LEFT | MOTOR_RIGHT);
            break;
         case 'w': // Forward
            MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);
            printf("> [Motor] Forward \n");
            break;
        case 's': // Reverse
            MOTOR_setDirection(MOTOR_DIR_REVERSE, MOTOR_LEFT | MOTOR_RIGHT);
            printf("> [Motor] Reverse \n");
            break;
        case 'a': // Left turn (in place)
            MOTOR_spotTurnPID(state.leftPID, state.rightPID, MOTOR_TURN_ANTICLOCKWISE, 90);
            printf("> [Motor] Left Turn (PID) \n");
            break;
        case 'd': // Right turn (in place)
            MOTOR_spotTurnPID(state.leftPID, state.rightPID, MOTOR_TURN_CLOCKWISE, 90);
            printf("> [Motor] Right Turn (PID) \n");
            break;
        case 'x': // Print wheel speed
            printf("> [Encoder] Wheel Speed: %.2f \n", ENCODER_getWheelSpeed(ENCODER_LEFT));
            break;
        }
    }
}