#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/datetime.h"

#include "motor/motor.h"
#include "motor/pid.h"
#include "encoder/encoder.h"
#include "lwip/apps/httpd.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "wifi/lwipopts.h"

#include "wifi/cgi.h"
#include "irline_barcode/infraredMain.c"
#include "line_detector/line_detector.h"
#include "ultrasonic/ultrasonic.h"

#define PID_KpL 0.15f
#define PID_KiL 0.f
#define PID_KdL 0.5f

#define PID_Kp 0.f
#define PID_Ki 0.1f
#define PID_Kd 0.f

uint trigPin = 16;
uint echoPin = 17;

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

    cyw43_arch_init();

    cyw43_arch_enable_sta_mode();

    // Connect to the WiFI network - loop until connected
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0)
    {
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected! \n");

    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");

    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

    State state;

    // Create PID contoller for motors
    state.rightPID = PID_create(PID_Kp, PID_Ki, PID_Kd, 0, 0.f, 100.f);
    state.leftPID = PID_create(PID_KpL, PID_KiL, PID_KdL, 0, 0.f, 100.f);

    // Initialize modules
    MOTOR_init(state.leftPID, state.rightPID);
    ENCODER_init();
    LINEDETECTOR_Init();
    setupUltrasonicPins(trigPin, echoPin);
    // INFARED_init;

    state.bootTime = get_absolute_time();
    state.usePid = 1;
    add_repeating_timer_ms(-200, pidCallback, &state, &state.pidTimer);

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
        case 'x': // Print wheel speed
            printf("> [Encoder] Wheel Speed: %.2f \n", ENCODER_getWheelSpeed(ENCODER_LEFT));
            break;
        case 'o': // Decrease I factor by 0.1
            state.leftPID->kI -= 0.1f;
            printf("> [PID] LEFT I Factor: %.2f \n", state.leftPID->kI);
            break;
        case 'p': // Increase I factor by 0.1
            state.leftPID->kI += 0.1f;
            printf("> [PID] LEFT I Factor: %.2f \n", state.leftPID->kI);
            break;
        case 'u': // Decrease P factor by 0.1
            state.leftPID->kP -= 0.1f;
            printf("> [PID] LEFT P Factor: %.2f \n", state.leftPID->kP);
            break;
        case 'i': // Increase P factor by 0.1
            state.leftPID->kP += 0.1f;
            printf("> [PID] LEFT P Factor: %.2f \n", state.leftPID->kP);
            break;
        case '[': // Decrease D factor by 0.1
            state.leftPID->kD -= 0.1f;
            printf("> [PID] LEFT D Factor: %.2f \n", state.leftPID->kD);
            break;
        case ']': // Increase D factor by 0.1
            state.leftPID->kD += 0.1f;
            printf("> [PID] LEFT D Factor: %.2f \n", state.leftPID->kD);
            break;
        case 'O': // Decrease I factor by 0.1
            state.rightPID->kI -= 0.1f;
            printf("> [PID] RIGHT I Factor: %.2f \n", state.rightPID->kI);
            break;
        case 'P': // Increase I factor by 0.1
            state.rightPID->kI += 0.1f;
            printf("> [PID] RIGHT I Factor: %.2f \n", state.rightPID->kI);
            break;
        case 'U': // Decrease P factor by 0.1
            state.rightPID->kP -= 0.001f;
            printf("> [PID] RIGHT P Factor: %.3f \n", state.rightPID->kP);
            break;
        case 'I': // Increase P factor by 0.1
            state.rightPID->kP += 0.001f;
            printf("> [PID] RIGHT P Factor: %.3f \n", state.rightPID->kP);
            break;
        case '{': // Decrease D factor by 0.1
            state.rightPID->kD -= 0.1f;
            printf("> [PID] RIGHT D Factor: %.2f \n", state.rightPID->kD);
            break;
        case '}': // Increase D factor by 0.1
            state.rightPID->kD += 0.1f;
            printf("> [PID] RIGHT D Factor: %.2f \n", state.rightPID->kD);
            break;
        case 'r': // moving straight with PID controller for fixed distance 25cm
            MOTOR_moveFowardPID(state.leftPID, state.rightPID, 25);
            printf("> [Motor] Forward 25cm");
            break;
        case 't': // test turning and moving forward (movement of car in general)
            togglePid(&state);
            while (1)
            {
                printf("Move Straight\n");
                if (isLeftSensorLineDetected() == 1)
                {
                    printf("left detected\n");
                    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
                    MOTOR_spotTurn(MOTOR_TURN_CLOCKWISE, 6);
                    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
                }
                if (isRightSensorLineDetected() == 1)
                {
                    printf("right detected\n");
                    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
                    MOTOR_spotTurn(MOTOR_TURN_ANTICLOCKWISE, 6);
                    MOTOR_stop(MOTOR_LEFT | MOTOR_RIGHT);
                }
                else
                {
                    printf("move forward");
                    MOTOR_setDirection(MOTOR_DIR_FORWARD, MOTOR_LEFT | MOTOR_RIGHT);
                    MOTOR_setSpeed(70, MOTOR_LEFT);
                    MOTOR_setSpeed(68, MOTOR_RIGHT);
                }
            }
            break;
        case 'y': // test barcode while moving forward
            while (1)
            {
                printf("Move Straight\n");
                sleep_ms(500);
                if (isLineDetected())
                {
                    printf("Line detected\n");
                    uint8_t d = 'm';
                    switch (d)
                    {
                    case 'm':
                        togglePid(&state);
                        printf("case m\n");
                        sleep_ms(500);
                        d = 'n';
                    case 'n':
                        printf("case n\n");
                        while (true)
                        {
                            MOTOR_moveFoward(2);
                            if (readyToStartBarcodeState = true)
                            {
                                INFRARED_scanning();
                            }

                            // One char has been read, sort the timings
                            if (INFRARED_oneCharRead())
                            {
                                printf("\n\n ONE CHAR READ \n\n");
                                INFRARED_sortingTimings();
                            }

                            // Returning Char
                            if (INFRARED_readyToReturnChar())
                            {
                                const char *final_char = INFRARED_returnChar();
                                printf("\n\n YOUR FINAL READING IS ON THE FOLLOWING LINE: \033[1;31m%s\033[0m", final_char);

                                strcpy(final_reading, final_char);

                                INFRARED_resetForNewString(); // After getting final value, reset everything to be able to read a new char/string.
                                break;
                            }
                        }
                        break;
                    }
                }
                else
                {
                    MOTOR_moveFowardPID(state.leftPID, state.rightPID, 4);
                }
            }
            break;
        }
    }
}