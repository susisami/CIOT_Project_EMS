/* LIBRARIES */
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Config/config.h"
#include "pico/util/queue.h"


/* QUEUE DEFINITIONS */

    // QUEUE FOR ROT SW (GP12)
    queue_t button_queue;

    // QUEUE FOR OPTO FORK (GP28)
    queue_t opto_queue;

    // QUEUE FOR PIEZO SENSOR (GP27)
    queue_t pills_queue;


/* FUNCTION DEFINITIONS */

    // IRS FOR ROT SW (GP12), OPTO FORK (GP28) & PIEZO SENSOR (GP27)
    void interrupt_callback(uint gpio, uint32_t events)
    {
        bool value = true;

        if (gpio == OPTO_FORK) 
        {
            if (!gpio_get(OPTO_FORK)) {value = false;}
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