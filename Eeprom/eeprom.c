/*LIBRARIES*/
#include <stdio.h>
#include "../Dispense/run.h"
#include "../Interrupt/interrupt.h"
#include "../Initializes/initialize.h" // IWYU pragma: keep
#include "../Utilities/utils.h"
#include "eeprom.h"
#include "hardware/i2c.h"


/* FUNCTIONS */

/* MAIN EEPROM READING & SAVING */

/* LOAD UP SYSTEM SETTINGS */
int load_eeprom_settings(struct SystemInformation* systemVariables)
{
    // READ EEPROM DATA TO THE STRUCT
    read_program_state(&systemVariables->program_state);
    read_avg_steps(&systemVariables->avg_steps);
    read_opto_gap_steps(&systemVariables->opto_gap_steps);
    read_dispenser_position(&systemVariables->dispenser_position);
    read_movement(&systemVariables->isRunning);
    read_dispensed_pills(&systemVariables->dispensed_pills);

    if (systemVariables->program_state != PRE_CALIB)
    {
        if (systemVariables->program_state != CALIB && systemVariables->isRunning)
        {
            recover_prev_position(systemVariables);
        }

        printf("[!] System started with previous settings [!]\n");
    }

    else
    {
        printf("[!] System started with default settings [!]\n");
    }

    return 0;
}

// RECOVER PREVIOUS DISPENSE POSITION IF wasRunning = true
int recover_prev_position(const struct SystemInformation* systemVariables)
{
    bool edgeDetected;
    uint carousel_speed = CAROUSEL_DEF_SPEED;

    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL, true);

    while (!queue_try_remove(&opto_queue, &edgeDetected))
    {
        stepper_motor_run(COUNTERCLOCKWISE, &carousel_speed,true);
    }

    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL, false);

    for (int i = 0; i < systemVariables->opto_gap_steps / 2; i++)
    {
        stepper_motor_run(COUNTERCLOCKWISE,  &carousel_speed,true);
    }

    run_motor(systemVariables->avg_steps, systemVariables->dispenser_position, true);

    return 0;
}

// Prints the system status [state, average step count, dispenser position + whether carousel was running when shutdown]
int print_system_status(const struct SystemInformation* systemVariables)
{
    for (int i = 0; i < 10; i++) { printf("\n"); }

    printf("[STATE]: %d\n", systemVariables->program_state);
    printf("[AVG-STEPS]: %d\n", systemVariables->avg_steps);
    printf("[OPTO-G-STEPS]: %d\n", systemVariables->opto_gap_steps);
    printf("[DISP-POS]: %d\n", systemVariables->dispenser_position);
    printf("[DISP-PILLS]: %d\n", systemVariables->dispensed_pills);
    printf("[RECALIB-MVMT]: %d\n", systemVariables->isRunning);

    return 0;
}

// Reset eeprom system variables
int eeprom_write_all()
{
    uint8_t termination_bf[3];
    termination_bf[2] = 0;

    for (uint address = 0; address < TOTAL_ADDRESSES; address++)
    {
        termination_bf[0] = (address >> 8) & 0xFF;
        termination_bf[1] = address & 0xFF;
        save_data(termination_bf, 3);
    }

    return 0;
}


/* READING OPERATIONS */

int read_program_state(program_state_t* program_state)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {
        EEPROM_ADDR_PROGRAM_STATE >> 8 & 0xFF, EEPROM_ADDR_PROGRAM_STATE & 0xFF
    };
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES / 2, memory_address);

    // printf("PROGRAM STATE [S1]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

    if (validate_state(data_array, MAX_READ_BYTES / 2))
    {
        *program_state = (program_state_t) data_array[0];
    }

    else { printf("Previous setting not found at [EEPROM: Program State]\n"); }

    return 0;
}

int read_movement(bool* isRunning)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {EEPROM_ADDR_DISPENSER_ON_MOVE >> 8 & 0xFF, EEPROM_ADDR_DISPENSER_ON_MOVE & 0xFF};
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES / 2, memory_address);
    // printf("Carousel Movement Detection [S2]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

    if (validate_state(data_array, MAX_READ_BYTES / 2))
    {
        *isRunning = data_array[0];
    }

    else { printf("Previous setting not found at [EEPROM: Carousel Movement Detection]\n"); }

    return 0;
}

int read_avg_steps(uint* avg_steps)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {EEPROM_ADDR_AVG_STEPS >> 8 & 0xFF, EEPROM_ADDR_AVG_STEPS & 0xFF};
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES, memory_address);
    // printf("Average Steps (1 / 2) [S3]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);
    // printf("Average Steps (2 / 2) [S3]: (NORMAL) %d | (INVERTED) %d\n", data_array[2], data_array[3]);

    if (validate_state(data_array, MAX_READ_BYTES))
    {
        *avg_steps = (data_array[0] << 8) | data_array[2];
    }

    else { printf("Previous setting not found at [EEPROM: Average Step Size]\n"); }

    return 0;
}

int read_opto_gap_steps(uint* opto_gap_steps)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {EEPROM_ADDR_GAP_STEPS >> 8 & 0xFF, EEPROM_ADDR_GAP_STEPS & 0xFF};
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES, memory_address);

    // printf("Opto Gap Steps (1 / 2) [S4]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);
    // printf("Opto Gap Steps (2 / 2) [S4]: (NORMAL) %d | (INVERTED) %d\n", data_array[2], data_array[3]);

    if (validate_state(data_array, MAX_READ_BYTES))
    {
        *opto_gap_steps = (data_array[0] << 8) | data_array[2];
    }
    else { printf("Previous setting not found at [EEPROM: Opto Gap Steps]\n"); }

    return 0;
}

int read_dispenser_position(uint* dispenser_position)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {
        EEPROM_ADDR_DISPENSER_POSITION >> 8 & 0xFF, EEPROM_ADDR_DISPENSER_POSITION & 0xFF
    };
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES / 2, memory_address);
    // printf("Dispenser Position [S5]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

    if (validate_state(data_array, MAX_READ_BYTES / 2))
    {
        *dispenser_position = data_array[0];
    }
    else { printf("Previous setting not found at [EEPROM: Dispenser Position]\n"); }

    return 0;
}

// Reads the amount of pills that was dispensed in the previous runs in total
int read_dispensed_pills(uint* dispensed_pills)
{
    const uint8_t memory_address[ADDRESS_BYTES] = {
        EEPROM_ADDR_DISPENSED_PILLS >> 8 & 0xFF, EEPROM_ADDR_DISPENSED_PILLS & 0xFF
    };
    uint8_t data_array[MAX_READ_BYTES];

    read_data(data_array, MAX_READ_BYTES / 2, memory_address);
    // printf("Dispensed Pills [S6]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

    if (validate_state(data_array, MAX_READ_BYTES / 2))
    {
        *dispensed_pills = data_array[0];
    }
    else { printf("Previous setting not found at [EEPROM: Dispensed Pills]\n"); }

    return 0;
}


/* WRITING OPERATIONS */

int write_eeprom(const uint16_t address, const uint data)
{
    payload_control_t payload;
    payload.data_length = count_bytes(data);
    payload.data_length *= 2; // original and inverted values

    if (payload.data_length == WRITE_BYTES_MIN)
    {
        payload.data_array[0] = (uint8_t)data;
        payload.data_array[1] = ~payload.data_array[0];
    }
    else
    {
        payload.data_array[0] = (uint16_t)data >> 8;
        payload.data_array[1] = ~payload.data_array[0];
        payload.data_array[2] = (uint16_t)data & 0xFF;
        payload.data_array[3] = ~payload.data_array[2];
    }

    // printf("NOR (1): [%d] | INV: [%d]", payload.data_array[0], payload.data_array[1]);
    // printf("NOR (2): [%d] | INV: [%d]", payload.data_array[2], payload.data_array[3]);

    package_data(payload.data_array, payload.data_length, payload.payload_array, address);
    payload.payload_length = payload.data_length + ADDRESS_BYTES;

    if (save_data(payload.payload_array, payload.payload_length) != payload.payload_length)
    {
        printf("[ERR] Program Eeprom save failed at MEM_ADD: %X\n", address);
    }

    return 0;
}


/* GENERAL READING & SAVING FUNCTIONS */
// SAVE DATA TO EEPROM (BYTE PER MEMORY ADDRESS (0xXX) IN I2C_ADDR_EEPROM DEVICE)
uint save_data(const uint8_t* payload, const uint len)
{
    const uint bytes_written = i2c_write_blocking(I2C_PORT, I2C_ADDR_EEPROM, payload, len, false);
    eeprom_ack_polling_with_timeout(EEPROM_ACK_TIMEOUT_US);

    return bytes_written;
}


// READ DATA FROM A SPECIFIED DATA SLOT OF ADDR
uint read_data(uint8_t* data, const uint len, const uint8_t* addr)
{
    // SETS THE POINTER INSIDE THE EEPROM TO 16-BIT ADDRESS 0xFFFA (6th last)
    i2c_write_blocking(I2C_PORT, I2C_ADDR_EEPROM, addr, 2, true);

    // READS FROM THE MEM ADDRESS POINTED AT
    return i2c_read_blocking(I2C_PORT, I2C_ADDR_EEPROM, data, len, false);
}


/* UTILITY */
// VALIDATE EEPROM VALUES TO BE USER INPUT
bool validate_state(const uint8_t* array, const int array_length)
{
    for (uint i = 0; i < array_length; i += 2)
    {
        // Inverts the uninverted value and compares it to the inverted value
        if ((uint8_t)~array[i] != array[i + 1])
        {
            return false;
        }
    }
    return true;
}

// FUNCTION TO TIE ALL GIVEN DATA (INVERTED & NORMAL) TOGETHER INTO A SINGLE TRANSMISSION BUFFER / PAYLOAD PACKAGE
uint package_data(const uint8_t* data_array, const uint data_array_length, uint8_t* payload_array,
                  const uint16_t memory_address)
{
    payload_array[0] = memory_address >> 8 & 0xFF;
    payload_array[1] = memory_address & 0xFF;

    for (int i = 0; i < data_array_length; i++)
    {
        payload_array[i + 2] = data_array[i];
    }

    return 0;
}


void eeprom_ack_polling_with_timeout(uint timeout_us)
{
    uint8_t dummy = 0;
    int read_bytes = 1;
    uint32_t start_time_us = time_us_32();

    while ((time_us_32() - start_time_us) < timeout_us)
    {
        int result = i2c_read_timeout_us(I2C_PORT, I2C_ADDR_EEPROM, 
            &dummy, read_bytes, false, I2C_POLLING_TIMEOUT_US);

        if (result == read_bytes) 
        {
            printf("EEPROM_ACK_POLL_TIME: %d\n", time_us_32()-start_time_us);
            return;
        }
    }
}