#ifndef CONFIG_H
#define CONFIG_H

/* CONSTANTS */

    /* GPIO OUTPUTS */
        // COILS
        #define MTR_IN1 2
        #define MTR_IN2 3
        #define MTR_IN3 6
        #define MTR_IN4 13

        // LEDS
        #define LED_1 20
        #define LED_2 21
        #define LED_3 22


    /* GPIO INPUTS */
        #define OPTO_FORK 28
        #define ROT_SW 12
        #define PIEZO_SR 27


    /* UTILS */
        #define BYTE_MAX_VALUE 0xFF

    /* SYSTEM TIMEOUTS */
        #define LED_BLINK_SLOW_MS 1000
        #define LED_BLINK_FAST_MS 100
        #define MTR_SLEEP_US 1100 // sleep between each motor step
        #define DISPENSE_TIMEOUT_MS 1000 // 30 000 ms
        #define PIEZO_TIMEOUT_MS 90 // t = sqrt(2h / g) Physics formula for free fall [WORST SCENARIO] + 5ms
        #define PIEZO_DROP_TIMEOUT_MS 50
        #define LORA_JOIN_TIMEOUT_MS 1000 // 20 000??
        #define LORA_MSG_SEND_TIMEOUT_MS 5000


    /* ISR RELATED */
        #define OPTO_QUEUE_SIZE 1
        #define BTN_QUEUE_SIZE 1
        #define PIEZO_QUEUE_SIZE 5


    /* MOTOR_CALIBRATION & DISPENSE RUNS */
        #define MTR_COILS 4
        #define MTR_PHASE_AMOUNT 8
        #define DISPENSE_ROUNDS 7
        #define STEP_COUNT_SECTIONS 2 // the whole rotation divided by the two opto edges into two sections
        #define DISPENSER_WHEEL_DIVISOR 8 // dispenser wheel slots
        #define CALIBRATION_ROTATIONS 2 // how many rotations to count the average steps from
        #define NO_PILL_BLINK_TIMES 5


    /*LORAWAN */
        // HARDWARE
        #define LORA_UART_ID uart1
        #define LORA_BAUD_RATE 9600
        #define LORA_TX_PIN 4
        #define LORA_RX_PIN 5

        // NETWORK
        #define LORA_MAX_RETRY_ATTEMPTS 5
        #define LORA_DATA_RATE 5
        #define LORA_PORT 8

        // BUFFERS
        #define LORA_BUFFER_SIZE 128
        #define LORA_MESSAGE_SIZE 128
        #define LORA_COMMAND_SIZE 192


    /*I2C & EEPROM */
        // ADDRESSES
        #define TOTAL_ADDRESSES 16
        #define EEPROM_ADDR_PROGRAM_STATE 0x0000        // 0 - 1
        #define EEPROM_ADDR_AVG_STEPS 0x0002            // 2 - 5
        #define EEPROM_ADDR_GAP_STEPS 0x0006            // 6 - 9
        #define EEPROM_ADDR_DISPENSED_PILLS 0x000A      // 10 - 11
        #define EEPROM_ADDR_DISPENSER_POSITION 0x000C   // 12 - 13
        #define EEPROM_ADDR_DISPENSER_ON_MOVE 0x000E    // 14 - 15
        #define EEPROM_TOTAL_LOG_SAVES 0x0064           // 100 - 101

        // READS & WRITES
        #define MAX_PAYLOAD_SIZE 12 // DOUBLE THE BIGGEST POSSIBLE IN THIS PROGRAM
        #define TTL_DATA_BYTES 4
        #define ADDRESS_BYTES 2

        // I2C INITS
        #define I2C_FREQ 100000
        #define SDA_PIN 16
        #define SCL_PIN 17


#endif //CONFIG_H