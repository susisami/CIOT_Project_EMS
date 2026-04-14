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
    init_sys_variables(&systemVariables);    
    
    // INIT FUNCTIONS
    init_gpio_all();
    stdio_init_all();
    queue_init(&event_queue, sizeof(int), QUEUE_SIZE);
    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);


    printf("Boot\n");
    // MAIN PROGRAM LOOP
    while (true)
    {
        // read queue
        if (queue_try_remove(&event_queue, &irq_pin) && irq_pin == ROT_SW) {
            systemVariables.button_pressed = true;
        }

        switch (program_state) {

            case PRE_CALIB: // BLINK LED UNTIL BUTTON IS PRESSED

                if (systemVariables.button_pressed) // ROT_SW button pressed 
                {
                    while(queue_try_remove(&event_queue, &systemVariables.button_pressed));
                    systemVariables.button_pressed = false;
                    gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, false);
                    gpio_put(LED_2, false);
                    program_state = CALIB;
                } 
                else // BLINK LED
                { 
                    systemVariables.led_on = !systemVariables.led_on; 
                    gpio_put(LED_2, systemVariables.led_on);
                    sleep_ms(LED_BLINK_MS); 
                }
                break;


            case CALIB: // CALIBRATE MOTOR

                motor_calibration(&systemVariables);

                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);
                gpio_put(LED_2, true);
                program_state = PRE_DISPENSE;
                break;


            case PRE_DISPENSE: // WAIT FOR A BUTTON PRESS

                if (systemVariables.button_pressed) {
                    systemVariables.button_pressed = false;
                    gpio_put(LED_2, false);
                    program_state = DISPENSE;
                }
                break;


            case DISPENSE: // DISPENSE PILLS

/*                while (absolute_time_diff_us() && dispense_position < DISPENSE_COMPLETE) {


                    dispense_position++;
                }
*/

                init_sys_variables(&systemVariables); // init variables for a fresh start
                program_state = PRE_CALIB;
                break;
        }
    }
}