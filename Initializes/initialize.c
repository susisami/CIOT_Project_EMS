#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Macros/macros.h" 
#include "pico/util/queue.h"


// Interrupt callback function for  ROT_SW  &&  OPTO_FORK
    queue_t event_queue;
    absolute_time_t last_press_time;

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

    }

// Init GPIOs
    void init_gpio_all(void)
    {
        // LEDS
        gpio_init(LED_1);
        gpio_init(LED_2);
        gpio_init(LED_3);
        gpio_set_dir(LED_1, GPIO_OUT);
        gpio_set_dir(LED_2, GPIO_OUT);
        gpio_set_dir(LED_3, GPIO_OUT);
        gpio_put(LED_1, false); 
        gpio_put(LED_2, true); 
        gpio_put(LED_3, false); 

        // ROT_SW
        gpio_init(ROT_SW);
        gpio_set_dir(ROT_SW, GPIO_IN);
        gpio_pull_up(ROT_SW);
        
        // OPTO_FORK
        gpio_init(OPTO_FORK);
        gpio_set_dir(OPTO_FORK, GPIO_IN);
        gpio_pull_up(OPTO_FORK);

        // MOTOR
        gpio_init(MTR_IN1);
        gpio_init(MTR_IN2);
        gpio_init(MTR_IN3);
        gpio_init(MTR_IN4);
        gpio_set_dir(MTR_IN1, GPIO_OUT);
        gpio_set_dir(MTR_IN2, GPIO_OUT);
        gpio_set_dir(MTR_IN3, GPIO_OUT);
        gpio_set_dir(MTR_IN4, GPIO_OUT);
    }

