/* LIBRARIES */
    // STD HEADER FILES
    #include <stdio.h>
    #include <stdbool.h>
    #include <sys/unistd.h>

    // OWN HEADER FILES
    #include "hardware/pwm.h" // IWYU pragma: keep
    #include "pico/stdlib.h" // IWYU pragma: keep
    #include "pico/util/queue.h"
    #include "Config/config.h"
    #include "Initializes/initialize.h"
    #include "Calibration/calib.h"
    #include "Interrupt/interrupt.h"
    #include "Dispense/dispense.h"
    #include "Eeprom/eeprom.h"


/* MAIN */
int main() {
    /* SYSTEM STRUCTURES */
        // System Information:
        sys_info_t systemVariables;

        // LoRaWan handling:
        lora_module_t lora_module;


    /* INITIALIZE FUNCTIONS */
    stdio_init_all();
    init_gpio_all();
    init_i2c_instance();
    init_sys_variables(&systemVariables);
    init_lora(&lora_module);

    // you can test lora messages even not connected to server
    //uncomment
    lora_module.state = LORA_READY;
    lora_module.joined = true;


    /* QUEUE INITIALIZES */
        //ROTARY ENCODER SW QUEUE (GP12)
        queue_init(&button_queue, sizeof(bool), BTN_QUEUE_SIZE);

        // OPTO FORK (GP28)
        queue_init(&opto_queue, sizeof(bool), OPTO_QUEUE_SIZE);

        // PIEZO SENSOR (GP27)
        queue_init(&piezo_queue, sizeof(bool), PIEZO_QUEUE_SIZE);


    /* ISR ENABLES */
    gpio_set_irq_enabled_with_callback(ROT_SW, GPIO_IRQ_EDGE_FALL, true, &interrupt_callback);
    gpio_set_irq_enabled_with_callback(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false, &interrupt_callback);


    /* READ PREVIOUS SETTINGS & START SYSTEM CONTROL PROGRAM */
    load_eeprom_settings(&systemVariables);
    print_system_status(&systemVariables);
    lora_power_loss_event(&lora_module, &systemVariables);


    while (true)
    {
        switch (systemVariables.program_state)
        {
            case PRE_CALIB: // BLINK LED UNTIL BUTTON IS PRESSED //

                if (systemVariables.button_pressed) // ROT_SW button pressed
                {
                    systemVariables.button_pressed = false;
                    gpio_put(LED_2, false);

                    systemVariables.program_state = CALIB;
                    write_eeprom(EEPROM_ADDR_PROGRAM_STATE, systemVariables.program_state);

                }
                else // BLINK LED
                {
                    gpio_put(LED_2, !gpio_get(LED_2));

                    // sleep LED_BLINK_MS, stop sleeping if queue is not empty (== button has been pressed)
                    for (int i = 0; i < LED_BLINK_SLOW_MS && !queue_try_peek(&button_queue, NULL); i++)
                    {
                        sleep_ms(1);
                    }

                    systemVariables.program_state = PRE_CALIB;
                }
                break;


            case CALIB: // CALIBRATE MOTOR //

                // CALIBRATION RUN
                systemVariables.isRunning = true;
                write_eeprom(EEPROM_ADDR_DISPENSER_ON_MOVE, systemVariables.isRunning);
                motor_calibration(&systemVariables.avg_steps, &systemVariables.opto_gap_steps);
                systemVariables.isRunning = false;
                write_eeprom(EEPROM_ADDR_DISPENSER_ON_MOVE, systemVariables.isRunning);

                // AFTER CALIBRATION RUN
                systemVariables.program_state = PRE_DISPENSE;
                write_eeprom(EEPROM_ADDR_PROGRAM_STATE, systemVariables.program_state);
                write_eeprom(EEPROM_ADDR_AVG_STEPS, systemVariables.avg_steps);
                write_eeprom(EEPROM_ADDR_GAP_STEPS, systemVariables.opto_gap_steps);

                print_system_status(&systemVariables);

                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);

                break;


            case PRE_DISPENSE: // WAIT FOR A BUTTON PRESS //

                if (systemVariables.button_pressed)
                {
                    systemVariables.program_state = DISPENSE;
                    write_eeprom(EEPROM_ADDR_PROGRAM_STATE, systemVariables.program_state);

                    gpio_put(LED_2, false);
                    systemVariables.button_pressed = false;
                }
                else
                {
                    gpio_put(LED_2, true);
                    systemVariables.program_state = PRE_DISPENSE;
                }
                break;


            case DISPENSE: // DISPENSE PILLS //

                dispense(&systemVariables, &lora_module);

                lora_send_event(&lora_module, EVENT_DISPENSER_EMPTY, NULL);

                systemVariables.program_state = RESET;
                write_eeprom(EEPROM_ADDR_PROGRAM_STATE, systemVariables.program_state);

                break;


            case RESET: // RESET //
            
                // Reset the systemVariables and write reset values to EEPROM
                init_sys_variables(&systemVariables);

                eeprom_write_all();

                print_system_status(&systemVariables);

                lora_send_event(&lora_module, EVENT_RESET, NULL);


                gpio_set_irq_enabled(ROT_SW, GPIO_IRQ_EDGE_FALL, true);
                
                systemVariables.program_state = PRE_CALIB;
                break;

        }

        queue_try_remove(&button_queue, &systemVariables.button_pressed);
    }
}