#ifndef MACROS_H
#define MACROS_H

/* CONSTANTS */

    // GPIO OUTPUTS
        // COILS
        #define MTR_IN1 2
        #define MTR_IN2 3
        #define MTR_IN3 6
        #define MTR_IN4 13

        // LEDS
        #define LED_1 20
        #define LED_2 21
        #define LED_3 22


    // GPIO INPUTS
    #define OPTO_FORK 28
    #define ROT_SW 12
    #define PIEZO_SR 27

    // TIMEOUTS
    #define LED_BLINK_SLOW_MS 1000
    #define LED_BLINK_FAST_MS 100
    #define MTR_SLEEP_US 1100 // sleep between each motor step
    #define DISPENSE_TIMEOUT_MS 1000 // 30 000 ms
    #define PIEZO_TIMEOUT_MS 90 // t = sqrt(2h / g) Physics formula for free fall [WORST SCENARIO] + 5ms

    // INTERRUPTS
    #define QUEUE_SIZE 10

    // MOTOR_CALIBRATION & DISPENSE RUNS
    #define MTR_COILS 4
    #define MTR_PHASE_AMOUNT 8
    #define DISPENSE_ROUNDS 7
    #define DIVIDE_ROTATION 2 // the whole rotation divided by the two opto edges into two sections
    #define DISPENSER_WHEEL_DIVISOR 8 // dispenser wheel slots
    #define CALIBRATION_ROTATIONS 2 // how many rotations to count the average steps from

    // LORA HARDWARE
    #define LORA_UART_ID uart1
    #define LORA_BAUD_RATE 9600
    #define LORA_TX_PIN 4
    #define LORA_RX_PIN 5

    // TIMEOUTS
    #define LORA_JOIN_TIMEOUT_MS 20000
    #define LORA_MSG_SEND_TIMEOUT_MS 5000

    // NETWORK
    #define LORA_MAX_RETRY_ATTEMPTS 5
    #define LORA_DATA_RATE 5
    #define LORA_PORT 8

    // BUFFERS
    #define LORA_BUFFER_SIZE 128
    #define LORA_MESSAGE_SIZE 128
    #define LORA_COMMAND_SIZE 192

    // I2C
    #define EEPROM_ADDR_PROGRAM_STATE 0x0000
    #define EEPROM_ADDR_DISPENSER_POSITION 0x0002
    #define EEPROM_ADDR_AVG_STEPS 0x0004 // 2 bytes 
    #define EEPROM_ADDR_DISPENSER_IS_CALIBRATED 0x0008
    #define EEPROM_ADDR_DISPENSER_ON_MOVE 0x00010
    #define I2C_FREQ 100000
    #define SDA_PIN 16
    #define SCL_PIN 17

#endif //MACROS_H