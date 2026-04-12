/* LIBRARIES */
    // STD C  FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // HEADER FILES
    #include "hardware/pwm.h"
    #include "pico/stdlib.h"
    #include "pico/util/queue.h"
    #include "calib.h"

/* CONSTANTS */
    // ISR CONSTANTS
    #define DEBOUNCE_MS 300

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
    bool isCalibrated = false;

    stdio_init_all();
    while (true)
    {
        while (queue_try_remove() && !isCalibrated)
        {

            isCalibrated = true;
        }

        while (queue_try_remove() && isCalibrated)
        {

        }
    }
}

//ISRS

    // ROTARY ENCODER SW
    void irq_rot_sw(uint gpio, uint32_t events)
    {
        absolute_time_t start_time = get_absolute_time();

        if (absolute_time_diff_us(last_press_time, start_time) > DEBOUNCE_MS * 1000)
        {
            last_press_time = start_time;

            queue_try_add()
        }
    }