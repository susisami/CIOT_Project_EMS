/* LIBRARIES */
    // STD HEADER FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // OWN HEADER FILES
    #include "hardware/pwm.h"
    #include "pico/stdlib.h"
    #include "pico/util/queue.h"
    #include "functions.h"

/* CONSTANTS */
    // SYSTEM VARIABLES
    #define DISPENSE_COMPLETE 7

    // LED GPIOS
    #define LED_1 20
    #define LED_2 21
    #define LED_3 22
    #define SLEEP_MS 250

    // INPUT GPIOS
    #define ROT_SW 12

    // ISR CONSTANTS
    #define DEBOUNCE_MS 300


/* ENUMS */
    // PROGRAM STATES
    typedef enum {PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE} program_state_t;


/* FUNCTION DECLARATIONS */

    // MAIN FUNCTIONS

    // ISR

    // UTILITIES

/* GLOBAL VARIABLES */

    // ISR TIMEOUTS
    static absolute_time_t last_press_time;

void irq_rot_sw(uint gpio, uint32_t events);


/* SYSTEM QUEUES */
static queue_t event_queue;


/* FUNCTIONS */

int main() {
    /* SYSTEM VARIABLES */
        program_state_t program_state = PRE_CALIB;
        uint dispense_position = 0;
        bool isCalibrated = false;
        bool pressed = false;

        // CALIBRATED VARIABLES
        uint avg_steps = 0;


    // INIT FUNCTIONS
    gpio_set_irq_enabled_with_callback();

    stdio_init_all();
    while (true)
    {
        queue_try_remove(&event_queue, &pressed);

        switch (program_state) {
            case PRE_CALIB:

                sleep_ms(SLEEP_MS);
                gpio_put(LED_2, 1);
                sleep_ms(SLEEP_MS);
                gpio_put(LED_2, 0);

                if (pressed) {
                    program_state = CALIB;
                    pressed = false;
                }

                break;
            case CALIB:
                    // CALIB FUNCTION
                    program_state = PRE_DISPENSE;
                    gpio_put(LED_2, 1);
                break;

            case PRE_DISPENSE:

                if (pressed) {
                    gpio_put(LED_2, 0);
                    program_state = DISPENSE;
                    pressed = false;
                }
                break;

            case DISPENSE:
                while (absolute_time_diff_us() && dispense_position < DISPENSE_COMPLETE) {


                    dispense_position++;
                }

                program_state = PRE_CALIB;

                break;
        }
    }
}

//ISRS

    // ROTARY ENCODER SW
    void irq_rot_sw(uint gpio, uint32_t events)
    {
        const absolute_time_t start_time = get_absolute_time();

        if (absolute_time_diff_us(last_press_time, start_time) > DEBOUNCE_MS * 1000)
        {
            last_press_time = start_time;

            const bool press = true;
            queue_try_add(&event_queue, &press);
        }
    }