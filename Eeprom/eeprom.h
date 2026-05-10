#ifndef EEPROM_H
#define EEPROM_H

/* LIBRARIES */

// CUSTOM LIBRARIES
#include "../Initializes/initialize.h"


/* FUNCTIONS */

/* MAIN EEPROM LOGIC FUNCTIONS */

/* EEPROM OVERALL MANAGEMENT OPERATIONS */
// LOAD UP SYSTEM'S SETTINGS
int load_eeprom_settings(struct SystemInformation* systemVariables);

// RECOVER PREVIOUS DISPENSE POSITION IF wasRunning = true
int recover_prev_position(const struct SystemInformation* systemVariables);

// Prints the system status [state, average step count, dispenser position + whether carousel was running when shutdown]
int print_system_status(const struct SystemInformation* systemVariables);

// RESETS SYSTEM VARIABLES' ADDRESSES INSIDE EEPROM
int eeprom_write_all();


/* READING OPERATIONS */
// SEARCH FOR PROGRAM STATE IN EEPROM
int read_program_state(program_state_t* program_state);

// SEARCH WHETHER THE SYSTEM WAS MOVING OR NOT WHEN TURNED OFF
int read_movement(bool* isRunning);

// SEARCH FOR AVG STEPS IN EEPROM
int read_avg_steps(uint* avg_steps);

// SEARCH FOR STEPS BETWEEN THE FALLING & RISING EDGE OF GP27
int read_opto_gap_steps(uint* opto_gap_steps);

// SEARCH FOR DISPENSER POSITIONS IN EEPROM
int read_dispenser_position(uint* dispenser_position);

// SEARCH FOR DISPENSER POSITIONS IN EEPROM
int read_dispensed_pills(uint* dispensed_pills);


/* WRITING OPERATIONS */

// Common write function for any system variable to be saved in the EEPROM
int write_eeprom(uint16_t address, uint data);


/* GENERAL READING & SAVING FUNCTIONS */
// SAVE DATA TO EEPROM (BYTE PER MEMORY ADDRESS (0xXX) IN 0x50 DEVICE)
uint save_data(const uint8_t* payload, uint len);

// READ DATA FROM A SPECIFIED DATA SLOT OF ADDR
uint read_data(uint8_t* data, uint len, const uint8_t* addr);


/* UTILITY */
// VALIDATE EEPROM VALUES TO BE USER INPUT
bool validate_state(const uint8_t* array, int array_length);

// FUNCTION TO TIE ALL GIVEN DATA (INVERTED & NORMAL) TOGETHER INTO A SINGLE TRANSMISSION BUFFER / PAYLOAD PACKAGE
uint package_data(const uint8_t* data_array, uint data_array_length, uint8_t* payload_array, uint16_t memory_address);


#endif //EEPROM_H
