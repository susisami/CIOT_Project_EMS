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
    // ISR
    void irq_rot_sw(uint gpio, uint32_t events);


/* GLOBAL VARIABLES */
    // ISR TIMEOUTS
    static absolute_time_t last_press_time;


/* STRUCTURES */
    // SYSTEM STRUCTURE
    typedef struct SystemInformation
    {
        bool isCalibrated;
        uint dispenser_position;
        uint avg_steps;
        uint steps_per_rev; // total steps per full cycle (approx. 4096)
        bool pressed;
        bool led_state;

    } sys_info_t;


/* SYSTEM QUEUES */
    static queue_t event_queue;


/* MAIN */
int main() {

    /* SYSTEM VARIABLES */
    program_state_t program_state = PRE_CALIB;

    sys_info_t systemVariables;
    systemVariables.pressed = false;
    systemVariables.led_state = false;
    systemVariables.isCalibrated = false;
    systemVariables.dispenser_position = 0;
    systemVariables.avg_steps = 0;
    systemVariables.steps_per_rev = 0;
    
    
    // INIT FUNCTIONS
    init_gpio_all();
    stdio_init_all();
    queue_init(&event_queue, sizeof(bool), QUEUE_SIZE);
    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &irq_rot_sw);


    printf("Boot\n");
    // MAIN PROGRAM LOOP
    while (true)
    {
        queue_try_remove(&event_queue, &systemVariables.pressed);


        switch (program_state) {

            case PRE_CALIB:

                // TOGGLE LED
                systemVariables.led_state = !systemVariables.led_state; 
                gpio_put(LED_2, systemVariables.led_state);
                sleep_ms(LED_BLINK_MS); // delays the calibration after button press, find a solution
                
                if (systemVariables.pressed) { 
                    program_state = CALIB;
                    systemVariables.pressed = false;
                    gpio_put(LED_2, false);
                }
                break;


            case CALIB:

                systemVariables.steps_per_rev = motor_calibration();

                // TODO: fix the callback-bug
                gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_RISE, true, &irq_rot_sw); // temporary fix, will be removed

                program_state = PRE_DISPENSE;
                gpio_put(LED_2, true);
                break;


            case PRE_DISPENSE:

                if (systemVariables.pressed) {
                    gpio_put(LED_2, 0);
                    program_state = DISPENSE;
                    systemVariables.pressed = false;
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
