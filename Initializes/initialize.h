#ifndef INITIALIZE_H
#define INITIALIZE_H
#include "pico/stdlib.h" // IWYU pragma: keep
#include "../LoRa/lora.h"



/* ENUMS */

    // PROGRAM STATES
    typedef enum { PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE } program_state_t;
    

/* STRUCTURES */

    // SYSTEM STRUCTURE
    typedef struct SystemInformation
    {
        // EEPROM VARIABLES

            // Current program state { PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE }
            program_state_t program_state;

            // total steps per full cycle (approx. 4096)
            uint avg_steps;

            // Marks the current total amount of steps in the mode DISPENSE
            uint dispenser_position;

            // Shows whether the motor was running / not running
            bool isRunning;

        // Determines when the system was calibrated in the program state { CALIB }
        bool isCalibrated;


        // Timestamp for controlling the dispense happening at intervals.
        absolute_time_t dispense_start_time;

        // Amount of pills that has been dispensed
        uint dispensed_pills;

        // 
        bool button_pressed;

        // 
        bool led_on;

    } sys_info_t;


    // EEPROM PAYLOAD STRUCTURE ( MADE FOR PROGRAM SCALABILITY )
    typedef struct Eeprom_Payload
    {
        uint8_t payload_array[MAX_PAYLOAD_SIZE];
        int payload_length;

        // Data to be stored inside the payload array
        uint8_t data_array[MAX_TTL_READS];
        int data_length;

    } payload_control_t;


/* FUNCTION DECLARATIONS */

    // init GPIOs
    void init_gpio_all(void);

    // init systemVariables
    void init_sys_variables(sys_info_t *systemVariables);

    // init LoRa module
    void init_lora(lora_module_t *lora_module);

    // init i2c
    int init_i2c_instance(void);

#endif //INITIALIZE_H