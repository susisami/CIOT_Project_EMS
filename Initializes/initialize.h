#ifndef INITIALIZE_H
#define INITIALIZE_H
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../LoRa/lora.h"

/* STRUCTURES */

    // SYSTEM STRUCTURE
    typedef struct SystemInformation
    {
        bool isCalibrated;

        // total steps per full cycle (approx. 4096)
        uint avg_steps;

        // position from 0 to steps_per_rev (calibrated=0, increasing clockwise)
        uint steps_per_rev;

        // Timestamp for controlling the dispense happening at intervals.
        absolute_time_t dispense_start_time;

        // Marks the current total amount of steps in the mode DISPENSE
        uint dispenser_position;

        // Amount of pills that has been dispensed
        uint dispensed_pills;

        // 
        bool button_pressed;

        // 
        bool led_on;

    } sys_info_t;

/* FUNCTION DECLARATIONS */

    // init GPIOs
    void init_gpio_all(void);

    // init systemVariables
    void init_sys_variables(sys_info_t *systemVariables);

    // init LoRa module
    void init_lora(lora_module_t *lora_module);


#endif //INITIALIZE_H