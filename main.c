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
    void interrupt_callback(uint gpio, uint32_t events);


/* GLOBAL VARIABLES */


/* STRUCTURES */



/* MAIN */
int main() {

    /* SYSTEM VARIABLES */
    program_state_t program_state = PRE_CALIB;
    int irq_pin;

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
    queue_init(&event_queue, sizeof(int), QUEUE_SIZE);
    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);


    printf("Boot\n");
    // MAIN PROGRAM LOOP
    while (true)
    {
        if (queue_try_remove(&event_queue, &irq_pin) && irq_pin == ROT_SW) {
            systemVariables.pressed = true;
        }


        switch (program_state) {

            case PRE_CALIB:

                if (systemVariables.pressed) // ROT_SW PRESSED 
                {
                    program_state = CALIB;
                    while(queue_try_remove(&event_queue, &systemVariables.pressed));
                    systemVariables.pressed = false;
                    gpio_put(LED_2, false);
                } 
                else // BLINK LED
                { 
                    systemVariables.led_state = !systemVariables.led_state; 
                    gpio_put(LED_2, systemVariables.led_state);
                    sleep_ms(LED_BLINK_MS); 
                }
                break;


            case CALIB:

                motor_calibration(&systemVariables);

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