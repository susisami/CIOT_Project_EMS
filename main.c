/* LIBRARIES */
    // STD HEADER FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // OWN HEADER FILES
    #include "hardware/pwm.h" // IWYU pragma: keep
    #include "pico/stdlib.h" // IWYU pragma: keep
    #include "pico/util/queue.h"
    #include "Macros/macros.h"
    #include "Initializes/initialize.h"
    #include "Calibration/calib.h"
    #include "Functions/functions.h" // IWYU pragma: keep
    #include "Dispense/run.h"
    #include "Eeprom/eeprom.h"



/* MAIN */
int main() {
    /* SYSTEM VARIABLES */
    sys_info_t systemVariables;
    payload_control_t payloadController;

    init_sys_variables(&systemVariables);

    int irq_pin;
    // INIT FUNCTIONS
    stdio_init_all();
    init_gpio_all();
    init_i2c_instance();


    queue_init(&button_queue, sizeof(bool), QUEUE_SIZE);
    queue_init(&pills_queue, sizeof(bool), QUEUE_SIZE);


    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);


    printf("Boot\n");


    lora_module_t lora_module;
    init_lora(&lora_module);


    // READ EEPROM FOR PREVIOUS SYSTEM SETTINGS
    load_eeprom_settings(&systemVariables);
    program_state_t program_state = systemVariables.program_state;

    // MAIN PROGRAM LOOP
    while (true)
    {
        // read queue
        queue_try_remove(&button_queue, &systemVariables.button_pressed);

        switch (program_state) {

            case PRE_CALIB: // BLINK LED UNTIL BUTTON IS PRESSED

                if (systemVariables.button_pressed) // ROT_SW button pressed 
                {
                    systemVariables.program_state = CALIB;
                    write_program_state(&systemVariables, &payloadController);
                    program_state = systemVariables.program_state;

                    systemVariables.button_pressed = false;
                    gpio_put(LED_2, false);

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

                // CALIBRATION RUN
                systemVariables.isRunning = true;
                write_movement(&systemVariables, &payloadController);
                motor_calibration(&systemVariables);
                systemVariables.isRunning = false;
                write_movement(&systemVariables, &payloadController);

                // AFTER CALIBRATION RUN
                systemVariables.program_state = PRE_DISPENSE;
                write_program_state(&systemVariables, &payloadController);
                write_avg_steps(&systemVariables, &payloadController);
                program_state = systemVariables.program_state;

                gpio_put(LED_2, 1);
                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);

                break;


            case PRE_DISPENSE: // WAIT FOR A BUTTON PRESS

                if (systemVariables.button_pressed) {

                    systemVariables.program_state = DISPENSE;
                    write_program_state(&systemVariables, &payloadController);
                    program_state = systemVariables.program_state;

                    systemVariables.button_pressed = false;
                    gpio_put(LED_2, false);

                }
                break;


            case DISPENSE: // DISPENSE PILLS

                while (systemVariables.dispenser_position < DISPENSE_ROUNDS) {
                    if (absolute_time_diff_us(systemVariables.dispense_start_time, get_absolute_time()) >= DISPENSE_TIMEOUT_MS * 1000)
                    {
                        bool dispensed = false;
                        systemVariables.dispense_start_time = get_absolute_time();
                        gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, true);

                        // EEPROM FUNCTIONALITY
                        systemVariables.isRunning = true;
                        write_movement(&systemVariables, &payloadController);
                        run_motor(systemVariables.avg_steps, 1);
                        systemVariables.isRunning = false;
                        write_movement(&systemVariables, &payloadController);

                        sleep_ms(PIEZO_TIMEOUT_MS);

                        while (queue_try_remove(&pills_queue, &dispensed)) systemVariables.dispensed_pills++;

                        if (!dispensed)
                        {
                           lora_send_event(&lora_module, EVENT_PILL_NOT_DISPENSED, NULL);
                           for (int i = 0; i < 5; i++)
                           {
                                gpio_put(LED_2, 1);
                                sleep_ms(LED_BLINK_FAST_MS);
                                gpio_put(LED_2, 0);
                                sleep_ms(LED_BLINK_FAST_MS);
                           }
                        }
                        else
                        {
                           lora_send_event(&lora_module, EVENT_PILL_DISPENSED, NULL);
                        }

                        gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false);

                        // EEPROM FUNCTIONALITY
                        systemVariables.dispenser_position++;
                        write_dispenser_position(&systemVariables, &payloadController);

                        dispensed = false;
                    }
                }

                lora_send_event(&lora_module, EVENT_DISPENSER_EMPTY, NULL);

                init_sys_variables(&systemVariables); // init variables for a fresh start

                // EEPROM FUNCTIONALITY //
                write_program_state(&systemVariables, &payloadController);
                write_avg_steps(&systemVariables, &payloadController);
                write_dispenser_position(&systemVariables, &payloadController);

                program_state = systemVariables.program_state;

                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);
                break;
        }
    }
}