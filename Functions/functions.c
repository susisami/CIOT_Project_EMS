/* LIBRARIES */
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Macros/macros.h"
#include "pico/util/queue.h"


/* QUEUES */

    // IRS QUEUES FOR ROT_SW, OPTO_FORK & PIEZO SENSOR
    queue_t event_queue;
    queue_t pills_queue;


/* GLOBAL VARIABLES */

    // STORAGE FOR DEBOUNCE TIME OF ROT SW
    absolute_time_t last_press_time;


/* FUNCTIONS */

    // IRS FOR ROT SW (GP12), OPTO FORK (GP28) & PIEZO SENSOR (GP27)
    void interrupt_callback(uint gpio, uint32_t events)
    {
        const absolute_time_t start_time = get_absolute_time();

        if (gpio == OPTO_FORK) // opto fork doesn't need debounce
        {
            queue_try_add(&event_queue, &gpio);
        }

        else if (gpio == ROT_SW && absolute_time_diff_us(last_press_time, start_time) > DEBOUNCE_MS * 1000)
        {
            last_press_time = start_time;

            queue_try_add(&event_queue, &gpio);
        }

        else if (gpio == PIEZO_SR)
        {
            const bool value = true;
            queue_try_add(&pills_queue, &value);
        }
    }