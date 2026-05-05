/* LIBRARIES */
    // STD LIBRARIES
    #include <stdio.h>

    // CUSTOM HEADERS
    #include "../Config/config.h"
    #include "../Interrupt/interrupt.h"
    #include "../Initializes/initialize.h"
    #include "../Eeprom/eeprom.h"
    #include "../LoRa/lora.h"
    #include "run.h"



/* FUNCTIONS */
void dispense(sys_info_t *systemVariables, payload_control_t *payloadController, lora_module_t *lora_module)
{
    while (systemVariables->dispenser_position < DISPENSE_ROUNDS) 
    {
        if (absolute_time_diff_us(systemVariables->dispense_start_time, 
                    get_absolute_time()) >= DISPENSE_TIMEOUT_MS * 1000)
        {
            bool dispensed = false;
            systemVariables->dispense_start_time = get_absolute_time();

            gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, true);

            // EEPROM FUNCTIONALITY-
            write_movement_state(payloadController, true);
            run_motor(systemVariables->avg_steps, 1);
            write_movement_state(payloadController, false);

            systemVariables->dispenser_position++;
            write_dispenser_position(systemVariables, payloadController);

            sleep_ms(PIEZO_TIMEOUT_MS);

            gpio_set_irq_enabled(PIEZO_SR, GPIO_IRQ_EDGE_FALL, false);

            while (queue_try_remove(&pills_queue, &dispensed)) { systemVariables->dispensed_pills++; }

            // EEPROM LOGIC FOR COUNTING THE TOTAL DISPENSED PILLS COMES HERE
            print_system_status(systemVariables);

            if (!dispensed)
            {
                lora_send_event(lora_module, EVENT_PILL_NOT_DISPENSED, NULL);
                for (int i = 0; i < NO_PILL_BLINK_TIMES*2; i++)
                {
                    gpio_put(LED_2, !gpio_get(LED_2));
                    sleep_ms(LED_BLINK_FAST_MS);
                }
            }
            else
            {
                lora_send_event(lora_module, EVENT_PILL_DISPENSED, NULL);
            }

            dispensed = false;
        }
    }
}
