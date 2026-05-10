#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Initializes/initialize.h" // IWYU pragma: keep
#include "../Interrupt/interrupt.h"
#include "../Config/config.h"
#include  "../Eeprom/eeprom.h"


/* ENUMS */
typedef enum
{
    WAIT_FIRST_EDGE,
    COUNT_FIRST_SECTION,
    COUNT_SECOND_SECTION,
    CALIBRATED
} calib_state_t;


/* FUNCTION PROTOTYPES */
void motor_calibration(uint* steps_per_rotation, uint* opto_gap_steps);
void motor_step(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int step_nr);
void position_correction(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int correction_steps);
void get_avg_steps(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[STEP_COUNT_SECTIONS]);


/* FUNCTIONS */

// MAIN CALIBRATION FUNCTION
/*
    1.  count average steps in two parts (opto gap  /  the rest)

    2.  center the dispenser wheel position
*/
void motor_calibration(uint* steps_per_rotation, uint* opto_gap_steps)
{
    // ENABLE INTERRUPT
    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

    // VARIABLES
    int steps[STEP_COUNT_SECTIONS];
    int steps_first_section = 0;
    int steps_second_section = 0;
    int correction_steps = 0;
    int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS] = {
        {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}
    };


    // GET AVERAGE STEPS
    get_avg_steps(mtr_steps_arr, steps);

    steps_first_section = steps[0];
    steps_second_section = steps[1];

    *steps_per_rotation = steps_first_section + steps_second_section;


    // CORRECT THE POSITION
    // smaller value = opto gap  ->  count correction steps
    if (steps_first_section < steps_second_section)
    {
        *opto_gap_steps = steps_first_section;
        correction_steps = steps_first_section / 2; // drive forward
    }
    else if (steps_first_section > steps_second_section)
    {
        *opto_gap_steps = steps_second_section;
        correction_steps = steps_second_section / -2; // reverse
    }

    position_correction(mtr_steps_arr, correction_steps);


    // DISABLE INTERRUPT
    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);

    // WRITE AVERAGE STEPS / REV & STEPS BETWEEN OPTO SENSOR EDGES TO EEPROM
    write_eeprom(EEPROM_ADDR_AVG_STEPS, *steps_per_rotation);
    write_eeprom(EEPROM_ADDR_GAP_STEPS, *opto_gap_steps);
}


// GET AVERAGE AMOUNT OF STEPS PER ROTATION
/*
    Divide full rotation into two segments/variables:
        "count_first_section"     (steps between    odd and even  -numbered opto edges)
        "count_second_section"    (steps between   even and odd   -numbered opto edges)

    Program flow:
        1. Rotate until the first opto_edge
        2. Count steps into the  "count_first_section"  until the second edge
        3. Between 2nd and 3rd edges count the steps into the "count_second_section"
        4. 3rd -> 4th edges count to  "count_first_section"  again
        5. Keep counting until enough full_rotations counted
        6. Count averages of each step count into results[]
*/
void get_avg_steps(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[STEP_COUNT_SECTIONS])
{
    int count_first_section = 0; // steps between    odd and even  -numbered opto edges
    int count_second_section = 0; // steps between   even and odd   -numbered opto edges
    int full_rotations = 0; // count the counted rotations
    bool junk;
    calib_state_t state = WAIT_FIRST_EDGE;


    while (state != CALIBRATED)
    {
        for (int step_nr = 0; step_nr < MTR_PHASE_AMOUNT; step_nr++)
        {
            motor_step(mtr_steps_arr, step_nr);

            bool opto_edge = queue_try_remove(&opto_queue, &junk);


            switch (state)
            {
            // ROTATE UNTIL THE FIRST EDGE
            case WAIT_FIRST_EDGE:
                if (opto_edge) state = COUNT_FIRST_SECTION;
                else state = WAIT_FIRST_EDGE;
                break;

            // Count steps between odd-numbered and even-numbered edges
            case COUNT_FIRST_SECTION:
                count_first_section++;

                if (opto_edge)
                {
                    state = COUNT_SECOND_SECTION;
                }
                else state = COUNT_FIRST_SECTION;

                break;

            // Count steps between even-numbered and odd-numbered edges
            //      calibrate given number of full_rotations
            case COUNT_SECOND_SECTION:
                count_second_section++;

                if (opto_edge)
                {
                    full_rotations++;

                    if (full_rotations < CALIBRATION_ROTATIONS) state = COUNT_FIRST_SECTION;
                    else state = CALIBRATED;
                }

                else state = COUNT_SECOND_SECTION;

                break;


            case CALIBRATED:
                break;
            }

            sleep_us(MTR_SLEEP_US);
        }
    }

    results[0] = (float)count_first_section / CALIBRATION_ROTATIONS;
    results[1] = (float)count_second_section / CALIBRATION_ROTATIONS;
}


// CORRECT THE POSITION
/*
Correcting/centering the dispenser wheel position during calibration by driving the motor the amount of correction_steps

int correction_steps:
    Positive value -> drive motor clockwise
    Negative value -> drive motor anti-clockwise
*/
void position_correction(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int correction_steps)
{
    if (correction_steps < 0) // ANTI-CLOCKWISE
    {
        correction_steps *= -1;

        for (int steps_left = correction_steps; steps_left > 0; steps_left--)
        {
            int step_nr = steps_left % 8;
            motor_step(mtr_steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }

    else // CLOCKWISE
    {
        for (int steps_left = 0; steps_left < correction_steps; steps_left++)
        {
            int step_nr = steps_left % 8;
            motor_step(mtr_steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }
}


// MOTOR DRIVERS
void motor_step(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int step_nr)
{
    gpio_put(MTR_IN1, mtr_steps_arr[step_nr][0]);
    gpio_put(MTR_IN2, mtr_steps_arr[step_nr][1]);
    gpio_put(MTR_IN3, mtr_steps_arr[step_nr][2]);
    gpio_put(MTR_IN4, mtr_steps_arr[step_nr][3]);
}
