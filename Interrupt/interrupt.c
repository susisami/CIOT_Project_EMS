/* LIBRARIES */

// CUSTOM HEADERS
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Config/config.h"
#include "pico/util/queue.h"


/* QUEUES */

// QUEUE FOR ROT SW (GP12)
queue_t button_queue;

// QUEUE FOR OPTO FORK (GP28)
queue_t opto_queue;

// QUEUE FOR PIEZO SENSOR (GP27)
queue_t piezo_queue;

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
        const absolute_time_t pill_drop = get_absolute_time();

        static absolute_time_t previous_pill_drop;

        if (absolute_time_diff_us(previous_pill_drop, pill_drop) > PIEZO_DROP_TIMEOUT_MS * 1000)
        // works with two pills & almost consistently with 3
        {
            previous_pill_drop = pill_drop;
            queue_try_add(&piezo_queue, &value);
        }
    }
}
