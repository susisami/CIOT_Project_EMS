#ifndef EEPROM_H
#define EEPROM_H

/* LIBRARIES */

    // CUSTOM LIBRARIES
    #include "../Initializes/initialize.h"


/* FUNCTIONS */

    /* MAIN EEPROM LOGIC FUNCTIONS */

        /* EEPROM OVERALL MANAGEMENT OPERATIONS */
            // LOAD UP SYSTEM'S SETTINGS
            int load_eeprom_settings(struct SystemInformation *systemVariables);

            // RECOVER PREVIOUS DISPENSE POSITION IF wasRunning = true
            int recover_prev_position (const struct SystemInformation *systemVariables);

            // Prints the system status [state, average step count, dispenser position + whether carousel was running when shutdown]
            int print_system_status (const struct SystemInformation *systemVariables);

            // RESETS SYSTEM VARIABLES' ADDRESSES INSIDE EEPROM
            int reset_system_variables(struct SystemInformation *systemVariables, struct Eeprom_Payload *payload);


        /* READING OPERATIONS */
            // SEARCH FOR PROGRAM STATE IN EEPROM
            int read_program_state (struct SystemInformation *systemVariables);

            // SEARCH WHETHER THE SYSTEM WAS MOVING OR NOT WHEN TURNED OFF
            int read_movement (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address);

            // SEARCH FOR AVG STEPS IN EEPROM
            int read_avg_steps (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address);

            // SEARCH FOR DISPENSER POSITIONS IN EEPROM
            int read_dispenser_position (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address);

            // SEARCH FOR DISPENSER POSITIONS IN EEPROM
            int read_dispensed_pills (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address);


        /* WRITING OPERATIONS */
            // Saves the program state to the EEPROM from systemVariables struct { PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE }
            int write_program_state (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload);

            // Saves the average step size to the EEPROM from systemVariables struct { 4096 +- 3 }
            int write_avg_steps (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload);

            // Saves the most recent dispenser position to the EEPROM from systemVariables struct { 0 - 7 }
            int write_dispenser_position (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload);

            // Saves the most recent dispenser position to the EEPROM from systemVariables struct { 0 - 7 }
            int write_dispensed_pills (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload);

            // Saves the state of movement during the CALIB or DISPENSE state { 0 = wasn't moving, 1 = was moving }
            int write_movement_state (struct SystemInformation *systemVariables, struct Eeprom_Payload *payload, bool isRunning);


        /* GENERAL READING & SAVING FUNCTIONS */
            // SAVE DATA TO EEPROM (BYTE PER MEMORY ADDRESS (0xXX) IN 0x50 DEVICE)
            int save_data(const uint8_t *packed_data, int len);

            // READ DATA FROM A SPECIFIED DATA SLOT OF ADDR
            int read_data(uint8_t *data, int len, const uint8_t *addr);


    /* UTILITY */
        // VALIDATE EEPROM VALUES TO BE USER INPUT
        bool validate_state(const uint8_t *array, int array_length);

        // FUNCTION TO TIE ALL GIVEN DATA (INVERTED & NORMAL) TOGETHER INTO A SINGLE TRANSMISSION BUFFER / PAYLOAD PACKAGE
        int package_data(const uint8_t *data_array, int data_array_length, uint8_t *payload_array, uint16_t memory_address);


#endif //EEPROM_H
