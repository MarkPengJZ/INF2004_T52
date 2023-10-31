#ifndef encoder_H_
#define encoder_H_

#define ENCODER_LEFT 1
#define ENCODER_RIGHT 2

void ENCODER_init(void);
float ENCODER_getWheelSpeed(int encoder);
int ENCODER_getWheelInterruptSpeed(int encoder);

void ENCODER_waitForISRInterrupts(uint target);
void ENCODER_alertAfterISRInterrupts(uint target, repeating_timer_callback_t callback);
int ENCODER_cmToSteps(float cm);

#endif