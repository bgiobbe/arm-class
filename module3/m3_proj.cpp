#include "mbed.h"

#define START_BUTTON p5
#define GUARD_SENSOR p6
#define TEMPERATURE_SENSOR p7
#define STOP_BUTTON p8

#define READY_LED p9
#define RUNNING_LED p10
#define GUARD_FAULT_LED p11
#define TEMPERATURE_FAULT_LED p12


void pulse_led(DigitalOut led);

// Define the Input pins
DigitalIn start(START_BUTTON);
DigitalIn stop(STOP_BUTTON);
DigitalIn guard_closed(GUARD_SENSOR);
DigitalIn overtemp(TEMPERATURE_SENSOR);

//Define the Output pins

// ready to run (flashing if false)
DigitalOut ready(READY_LED);
// motor is running
DigitalOut running(RUNNING_LED);
// guard opened while running
DigitalOut guard_fault(GUARD_FAULT_LED);    
// excess temperature while running
DigitalOut overtemp_fault(TEMPERATURE_FAULT_LED);


int main()
{
    while(1) {
        // clear all the LEDs
        ready = running = guard_fault = overtemp_fault = false;
        
        // wait until the stop button is released
        while (stop) {
            wait(0.25);
        }
        
        // NOT RUNNING state, blink the Ready LED
        while (!guard_closed || overtemp) {
            ready = !ready;
            wait(0.2);
        }
        ready = true;

        // READY TO RUN state
        while (!start && guard_closed && !overtemp) {
            wait(0.25);  // arbitrary time
        }
        running = (start && guard_closed && !overtemp && !stop);

        // RUNNING state
        while (running) {
            if (!guard_closed) {
                // stop running and pulse guard_fault led
                running = ready = false;
                pulse_led(guard_fault);
            } else if (overtemp) {
                // stop running and pulse overtemp_fault led
                running = ready = false;
                pulse_led(overtemp_fault);
            } else if (stop) {
                running = ready = false;
            } else {
                wait(0.25);  // arbitrary time
            }
        }
    } // end while(1)
}

// Turn the given LED on for 0.5s
void pulse_led(DigitalOut led) {
    led = true;
    wait(0.5);
    led = false;
}

