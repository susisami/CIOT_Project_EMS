#ifndef BLINK_RUN_H
#define BLINK_RUN_H
/* LIBRARIES */
#include "pico/util/queue.h" // IWYU pragma: keep


/* ENUMS */
    typedef enum { CLOCKWISE, COUNTERCLOCKWISE } run_direction_t;


/* FUNCTION DECLARATIONS */

    // RUNS THE CAROUSEL FORWARD FOR A GIVEN AMOUNT OF STEPS
    int run_motor (uint steps_per_rev, int times);

    // ACTIVATES ALL THE (4) COILS IN A SEQUENCE COMMANDED BY THE RUN MOTOR -FUNCTION
    int stepper_motor_run(uint direction);


#endif //BLINK_RUN_H