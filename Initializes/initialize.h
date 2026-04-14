#ifndef INITIALIZE_H
#define INITIALIZE_H
/* STRUCTURES */

    // SYSTEM STRUCTURE
    typedef struct SystemInformation
    {
        bool isCalibrated;

        uint avg_steps;
        // total steps per full cycle (approx. 4096)

        uint steps_per_rev;
        // position from 0 to steps_per_rev (calibrated=0, increasing clockwise)

        absolute_time_t dispense_start_time;
        // Timestamp for controlling the dispense happening at intervals.

        uint dispenser_position;
        // Marks the current total amount of steps in the mode DISPENSE

        uint dispensed_pills;
        // Amount of pills that has been dispensed

        bool button_pressed;
        //

        bool led_on;

    } sys_info_t;

/* FUNCTION DECLARATIONS */

    // init GPIOs
    void init_gpio_all(void);

    // init systemVariables
    void init_sys_variables(sys_info_t *systemVariables);


#endif //INITIALIZE_H