/* LIBRARIES */
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Macros/macros.h"
#include "pico/util/queue.h"


/* QUEUES */

    // IRS QUEUES FOR ROT_SW, OPTO_FORK & PIEZO SENSOR
    queue_t button_queue;
    queue_t opto_queue;
    queue_t pills_queue;


/* FUNCTIONS */

    // IRS FOR ROT SW (GP12), OPTO FORK (GP28) & PIEZO SENSOR (GP27)
    void interrupt_callback(uint gpio, uint32_t events)
    {
        const bool value = true;

        if (gpio == OPTO_FORK) 
        {
            queue_try_add(&opto_queue, &value);
        }

        else if (gpio == ROT_SW)
        {
            gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, false);
            queue_try_add(&button_queue, &value);
        }

        else if (gpio == PIEZO_SR)
        {
            queue_try_add(&pills_queue, &value);
        }
    }