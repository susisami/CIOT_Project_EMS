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
    #define LED_BLINK_MS 1000
    #define DEBOUNCE_MS 300
    #define MTR_SLEEP_US 1100
    #define DISPENSE_TIMEOUT_MS 5000 // 30 000 ms
    #define PIEZO_TIMEOUT_MS 90 // t = sqrt(2h / g) Physics formula for free fall [WORST SCENARIO] + 5ms

    // INTERRUPTS
    #define QUEUE_SIZE 10

    
    // MOTOR_CALIBRATION & DISPENSE RUNS
    #define MTR_COILS 4
    #define MTR_PHASE_AMOUNT 8
    #define MTR_CALIB_ROTATIONS 3
    #define DISPENSE_ROUNDS 7



#endif //MACROS_H