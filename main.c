/* LIBRARIES */
    // STD HEADER FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // OWN HEADER FILES
    #include "Dispense/run.h"
    #include "hardware/pwm.h" // IWYU pragma: keep
    #include "pico/stdlib.h" // IWYU pragma: keep
    #include "pico/util/queue.h"
    #include "Functions/functions.h" // IWYU pragma: keep
    #include "Macros/macros.h" 
    #include "Initializes/initialize.h"
    #include "Calibration/calib.h"


/* ENUMS */
    // PROGRAM STATES
    typedef enum { PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE } program_state_t;


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

    queue_init(&button_queue, sizeof(bool), QUEUE_SIZE);
    queue_init(&pills_queue, sizeof(bool), QUEUE_SIZE);

    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);

    printf("Boot\n");
    // MAIN PROGRAM LOOP
    while (true)
    {
        // read queue
        queue_try_remove(&button_queue, &systemVariables.button_pressed);

        switch (program_state) {

            case PRE_CALIB: // BLINK LED UNTIL BUTTON IS PRESSED

                if (systemVariables.button_pressed) // ROT_SW button pressed 
                {
                    systemVariables.button_pressed = false;
                    gpio_put(LED_2, false);

                    program_state = CALIB;
                } 
                else // BLINK LED
                { 
                    systemVariables.led_on = !systemVariables.led_on; 
                    gpio_put(LED_2, systemVariables.led_on);

                    // sleep LED_BLINK_MS, stop sleeping if queue is not empty (== button has been pressed)
                    for (int i = 0; i < LED_BLINK_SLOW_MS && !queue_try_peek(&button_queue, &irq_pin); i++)
                    {
                        sleep_ms(1); 
                    }
                }
                break;


            case CALIB: // CALIBRATE MOTOR

                motor_calibration(&systemVariables);
                gpio_put(LED_2, 1);
                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);

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

                while (systemVariables.dispenser_position < DISPENSE_ROUNDS) {
                    if (absolute_time_diff_us(systemVariables.dispense_start_time, get_absolute_time()) >= DISPENSE_TIMEOUT_MS * 1000)
                    {
                        bool dispensed = false;
                        systemVariables.dispense_start_time = get_absolute_time();
                        gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, true);
                        run_motor(systemVariables.steps_per_rev, 1);

                        sleep_ms(PIEZO_TIMEOUT_MS);

                        while (queue_try_remove(&pills_queue, &dispensed)) systemVariables.dispensed_pills++;

                        if (!dispensed)
                        {
                           for (int i = 0; i < 5; i++)
                           {
                                gpio_put(LED_2, 1);
                                sleep_ms(LED_BLINK_FAST_MS);
                                gpio_put(LED_2, 0);
                                sleep_ms(LED_BLINK_FAST_MS);
                           }
                        }

                        gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false);
                        systemVariables.dispenser_position++;
                        dispensed = false;
                    }
                }

                init_sys_variables(&systemVariables); // init variables for a fresh start
                program_state = PRE_CALIB;
                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);
                break;
        }
    }
}