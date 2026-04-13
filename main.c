/* LIBRARIES */
    // STD HEADER FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // OWN HEADER FILES
    #include "hardware/pwm.h" // IWYU pragma: keep
    #include "pico/stdlib.h" // IWYU pragma: keep
    #include "pico/util/queue.h"
    #include "Functions/functions.h" // IWYU pragma: keep
    #include "Macros/macros.h" 
    #include "Initializes/initialize.h"
    #include "Calibration/calib.h"

/* CONSTANTS */
    // SYSTEM VARIABLES
    #define DISPENSE_COMPLETE 7


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
    //        uint dispense_position = 0;
    //        bool isCalibrated = false;
    bool pressed = false;
    bool led_state = false;
    
    // CALIBRATED VARIABLES
    //        uint avg_steps = 0;
    
    
    // INIT FUNCTIONS
    gpio_init_all();
    stdio_init_all();
    queue_init(&event_queue, sizeof(bool), QUEUE_SIZE);
    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &irq_rot_sw);


    printf("Boot\n");
    while (true)
    {
        queue_try_remove(&event_queue, &pressed);

        switch (program_state) {

            case PRE_CALIB:

                // TOGGLE LED
                led_state = !led_state; 
                gpio_put(LED_2, led_state);
                sleep_ms(LED_BLINK_MS);
                
                if (pressed) {
                    program_state = CALIB;
                    pressed = false;
                    gpio_put(LED_2, false);
                }

                break;


            case CALIB:

                    calib();
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
/*                while (absolute_time_diff_us() && dispense_position < DISPENSE_COMPLETE) {


                    dispense_position++;
                }
*/
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