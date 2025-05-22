/*
 * Briefing
 *
 * You are developing the monitoring (but not control) system for a heating tank
 * in an industrial process. There are two sensors in the tank, one for 
 * temperature and the other for level. Four output states must be detected, each 
 * triggering a different audible status tone: 
 *
 *    Tank is full, temperature too high (upper third of temperature range):
 *        two-tone  
 *    Tank is full, temperature is good (middle third of temperature range):
 *        steady continuous tone 
 *    Tank is full, temperature too low (lower third of temperature range):
 *        single beeping tone 
 *    Tank level is too low (tank less than three quarters full), any temperature:
 *        frequency-modulated “warble”  tone, flashing LED 
 * 
 * Sensors must be tested at least once every two seconds. 
 *
 * Write a program to meet this requirement, testing and demonstrating it on the
 * Mbed simulator. Use potentiometers for tank level and temperature, and the
 *Pwm speaker to generate the audible status signals.
 */

#include "mbed.h"

// PIN ASSIGNMENTS
#define LED p5
#define LEVEL_POT p15
#define TEMP_POT p16
#define PWM_SPEAKER p21

#if !defined OFF && !defined ON
#define OFF 0
#define ON 1
#endif

// how often to check the inputs, in seconds
// (at least every 2 seconds)
#define SAMPLE_PERIOD 1.99

// Periods of tones in seconds
#define _200HZ 0.005
#define _500HZ 0.002
#define _1000HZ 0.001
#define INAUDIBLE 1.000

// volume/duty cycle (%)
#define VOLUME 0.1

// Delay betwwen tones in seconds
#define DELAY 1.0

// Globals
DigitalOut led(LED, OFF);
AnalogIn levelIn(LEVEL_POT);
AnalogIn temperatureIn(TEMP_POT);
PwmOut speaker(PWM_SPEAKER);

void twoTone(void);
void steadyTone(void);
void beepingTone(void);
void warbleAndFlash(void);

int main() {
    float level;
    float temperature;

    while (1) {
        level = levelIn.read();
        temperature = temperatureIn.read();
        if (level < 0.75) {
            printf("low level %f\n", level);
            warbleAndFlash();
        }
        else {
            led = OFF;
            printf("level OK\n");
            if (temperature > 0.666667) {
                twoTone();
            }
            else if (temperature < 0.333333) {
                beepingTone();
            }
            else {
                steadyTone();
            }
        }
    }
    return 0;
}

void beepingTone(void) {
    // 200 Hz and something inaudible
    speaker.period(INAUDIBLE);
    speaker = 0;
    wait(DELAY);
    speaker.period(_200HZ);
    speaker = VOLUME;
    wait(DELAY);
}

void steadyTone(void) {
    // 500 Hz
    speaker.period(_500HZ);
    speaker = VOLUME;
    wait(DELAY * 2);
}

void warbleAndFlash(void) {
    led = !led;
    speaker = VOLUME;
    // sweep starting at 100 Hz, ending at 500 Hz
    // takes about 0.102s
    for(float f=0.0; f<1.0; f+=0.05) { // 20 steps
        speaker.period(0.010 - (0.008*f));
        wait_ms (50);
    }
}

void twoTone(void) {
    // 200 Hz and 1000 Hz 
    speaker.period(_1000HZ);
    speaker = VOLUME;
    wait(DELAY);
    speaker.period(_200HZ);
    wait(DELAY);
}




