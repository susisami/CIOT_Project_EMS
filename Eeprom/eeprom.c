#include <stdio.h>

#include "../Dispense/run.h"
#include "../Functions/functions.h"
#include "../Initializes/initialize.h" // IWYU pragma: keep
#include "eeprom.h"
#include "hardware/i2c.h"

/* FUNCTIONS */

    /* MAIN EEPROM READING & SAVING */

        /* LOAD UP SYSTEM'S SETTINGS */
        int load_eeprom_settings(struct SystemInformation *systemVariables)
        {
            int edgeDetected;

            read_program_state(systemVariables);

            if (systemVariables->program_state != PRE_CALIB)
            {
                if (systemVariables->program_state != CALIB && systemVariables->isRunning)
                {
                    // LOGIC TO GO COUNTERCLOCKWISE UNTIL RISING EDGE AND BACKUP AVG STEPS / 16
                    while (!queue_try_remove(&opto_queue, &edgeDetected))
                    {
                        stepper_motor_run(CLOCKWISE);
                    }

                    printf("STEPS: %d\n", systemVariables->avg_steps);
                    printf("STEPS: %d\n", systemVariables->avg_steps);

                    for (int i = 0; i < systemVariables->avg_steps / 16; i++)
                    {
                        stepper_motor_run(COUNTERCLOCKWISE);
                    }
                }

                printf("[!] System started with previous settings.\n");
                printf ("[STE]: %d\n", systemVariables->program_state);
                printf ("[AVGS]: %d\n", systemVariables->avg_steps);
                printf ("[DPOS]: %d\n", systemVariables->dispenser_position);
                printf ("[MVMT]: %d\n", systemVariables->isRunning);
            }

            else
            {
                printf("[!] System started with default settings.\n");
            }

            return 0;
        }

        /* READING OPERATIONS */
            //
            int read_program_state (struct SystemInformation *systemVariables)
            {
                uint8_t memory_address[ADDRESS_BYTES];
                uint8_t data_array[MAX_TTL_READS];

                memory_address[0] = EEPROM_ADDR_PROGRAM_STATE >> 8 & 0xFF;
                memory_address[1] = EEPROM_ADDR_PROGRAM_STATE & 0xFF;

                read_data(data_array, MAX_TTL_READS / 2, memory_address);

                printf("PROGRAM STATE [S1]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

                if (validate_state(data_array, MAX_TTL_READS / 2))
                {
                    systemVariables->program_state = (program_state_t) data_array[0];
                    read_movement(systemVariables, data_array, memory_address);
                }

                else { printf("Validation error at [EEPROM: Program State]\n"); }

                return 0;
            }

            //
            int read_movement (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address)
            {
                memory_address[0] = EEPROM_ADDR_DISPENSER_ON_MOVE >> 8 & 0xFF;
                memory_address[1] = EEPROM_ADDR_DISPENSER_ON_MOVE & 0xFF;

                read_data(data_array, MAX_TTL_READS / 2, memory_address);
                printf("Carousel Movement Detection [S2]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

                if (validate_state(data_array, MAX_TTL_READS / 2))
                {
                    if (data_array[0] == 0)
                    {
                        printf("Carousel was not running, storing value [%d] to struct\n", data_array[0]);
                    }

                    else
                    {
                        printf("Carousel was running, storing value [%d] to struct\n", data_array[0]);
                    }

                    systemVariables->isRunning = data_array[0];
                    read_avg_steps(systemVariables, data_array, memory_address);
                }

                else { printf("Validation error at [EEPROM: Carousel Movement Detection]\n"); }

                return 0;
            }

            //
            int read_avg_steps (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address)
            {
                memory_address[0] = EEPROM_ADDR_AVG_STEPS >> 8 & 0xFF;
                memory_address[1] = EEPROM_ADDR_AVG_STEPS & 0xFF;

                read_data(data_array, MAX_TTL_READS, memory_address);
                printf("Average Steps (1 / 2) [S3]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);
                printf("Average Steps (2 / 2) [S3]: (NORMAL) %d | (INVERTED) %d\n", data_array[2], data_array[3]);

                if (validate_state(data_array, MAX_TTL_READS))
                {
                    systemVariables->avg_steps = (data_array[0] << 8) | data_array[2];
                    read_dispenser_position(systemVariables, data_array, memory_address);
                }

                else { printf("Validation error at [EEPROM: Average Step Size]\n"); }

                return 0;
            }

            //
            int read_dispenser_position (struct SystemInformation *systemVariables, uint8_t *data_array, uint8_t *memory_address)
            {
                memory_address[0] = EEPROM_ADDR_DISPENSER_POSITION >> 8 & 0xFF;
                memory_address[1] = EEPROM_ADDR_DISPENSER_POSITION & 0xFF;

                read_data(data_array, MAX_TTL_READS / 2, memory_address);
                printf("Dispenser Position [S4]: (NORMAL) %d | (INVERTED) %d\n", data_array[0], data_array[1]);

                if (validate_state(data_array, MAX_TTL_READS / 2))
                {
                    systemVariables->dispenser_position = data_array[0];
                }
                else { printf("Validation error at [EEPROM: Dispenser Position]\n"); }

                return 0;
            }



        /* WRITING OPERATIONS */
            // Saves the program state to the EEPROM from systemVariables struct { PRE_CALIB, CALIB, PRE_DISPENSE, DISPENSE }
            int write_program_state (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload)
            {
                payload->payload_length = 4;
                payload->data_length = 2;

                payload->data_array[0] = systemVariables->program_state;
                payload->data_array[1] = ~payload->data_array[0];

                package_data(payload->data_array, payload->data_length, payload->payload_array, EEPROM_ADDR_PROGRAM_STATE);

                const int operation_return = save_data(payload->payload_array, payload->payload_length);
                sleep_ms(5);

                if (operation_return == 4)
                {
                    printf("[OK] Program state save successful.\n");
                }

                else
                {
                    printf("[ERR] Program state save failed.\n");
                }

                return 0;
            }

            // Saves the average step size to the EEPROM from systemVariables struct { 4096 +- 3 }
            int write_avg_steps (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload)
            {
                payload->payload_length = 6;
                payload->data_length = 4;

                const uint16_t avg_steps_in_16_bits = systemVariables->avg_steps;

                // TWO BYTE NORMALS & INVERSIONS
                payload->data_array[0] = avg_steps_in_16_bits >> 8 & 0xFF;
                payload->data_array[1] = ~payload->data_array[0];
                payload->data_array[2] = avg_steps_in_16_bits & 0xFF;
                payload->data_array[3] = ~payload->data_array[2];

                package_data(payload->data_array, payload->data_length, payload->payload_array, EEPROM_ADDR_AVG_STEPS);

                const int operation_return = save_data(payload->payload_array, payload->payload_length);
                sleep_ms(5);

                if (operation_return == 6)
                {
                    printf("[OK] Average step size save successful.\n");
                }

                else
                {

                    printf("[ERR] Average step size save failed.\n");
                }
                return 0;
            }

            // Saves the most recent dispenser position to the EEPROM from systemVariables struct { 0 - 7 }
            int write_dispenser_position (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload)
            {
                payload->payload_length = 4;
                payload->data_length = 2;

                payload->data_array[0] = systemVariables->dispenser_position;
                payload->data_array[1] = ~payload->data_array[0];

                package_data(payload->data_array, payload->data_length, payload->payload_array, EEPROM_ADDR_DISPENSER_POSITION);

                const int operation_return = save_data(payload->payload_array, payload->payload_length);
                sleep_ms(5);

                if (operation_return == 4)
                {
                    printf("[OK] Dispenser position save successful.\n");
                }

                else
                {
                    printf("[ERR] Dispenser position save failed.\n");
                }

                return 0;
            }

            // Saves the state of movement during the CALIB or DISPENSE state { 0 = wasn't moving, 1 = was moving }
            int write_movement (const struct SystemInformation *systemVariables, struct Eeprom_Payload *payload)
            {
                payload->payload_length = 4;
                payload->data_length = 2;

                payload->data_array[0] = systemVariables->isRunning;
                payload->data_array[1] = ~payload->data_array[0];

                package_data(payload->data_array, payload->data_length, payload->payload_array, EEPROM_ADDR_DISPENSER_ON_MOVE);

                const int operation_return = save_data(payload->payload_array, payload->payload_length);
                sleep_ms(5);

                if (operation_return == 4)
                {
                    printf("[OK] System movement state save successful.\n");
                }

                else
                {
                    printf("[ERR] System movement state save failed.\n");
                }

                return 0;
            }

        /* GENERAL READING & SAVING FUNCTIONS */
            // SAVE DATA TO EEPROM (BYTE PER MEMORY ADDRESS (0xXX) IN 0x50 DEVICE)
            int save_data(const uint8_t *packed_data, const int len)
            {
                return i2c_write_blocking(i2c0, 0x50, packed_data, len, false);
            }


            // READ DATA FROM A SPECIFIED DATA SLOT OF ADDR
            int read_data(uint8_t *data, const int len, const uint8_t *addr)
            {
                // SETS THE POINTER INSIDE THE EEPROM TO 16-BIT ADDRESS 0xFFFA (6th last)
                i2c_write_blocking(i2c0, 0x50, addr, 2, true);
                sleep_ms(5);

                // READS FROM THE MEM ADDRESS POINTED AT
                return i2c_read_blocking(i2c0, 0x50, data, len, false);
            }


    /* UTILITY */
        // VALIDATE EEPROM VALUES TO BE USER INPUT
        bool validate_state(const uint8_t *array, const int array_length)
        {
            for (uint i = 0; i < array_length; i+=2)
            {
                // Inverts the uninverted value and compares it to the inverted value
                if ( (uint8_t) ~array[i] != array[i + 1])
                {
                    return false;
                }
            }

            return true;
        }

        // FUNCTION TO TIE ALL GIVEN DATA (INVERTED & NORMAL) TOGETHER INTO A SINGLE TRANSMISSION BUFFER / PAYLOAD PACKAGE
        int package_data(const uint8_t *data_array, const int data_array_length, uint8_t *payload_array, const uint16_t memory_address)
        {
            payload_array[0] = memory_address >> 8 & 0xFF;
            payload_array[1] = memory_address & 0xFF;

            for (int i = 0; i < data_array_length; i++)
            {
                payload_array[i + 2] = data_array[i];
            }

            return 0;
        }







/*              STATE                    TYPE            VALUE           ADDR        
            1. Program state            uint8_t         0-4 (enum)      0x0000      
            2. Dispenser position       uint8_t         0-7 (int)       0x0002      
            3. Average steps            uint16_t        ~4096 (int)     0x0004      
            4  Dispenser is moving      uint8_t         0-1 (bool)      0x0008      
*/




// Program state is written everytime it's changed


// Dispenser position is written everytime it finishes moving a 1/8 step 


// Average steps takes 2 bytes of memory and is written:    
//      after calibration: avg_steps 
//      when dispensing is done wite 0 to indicate that calibration haven't been done


// Dispenser is moving is written everytime dispenser starts moving and when it stops

