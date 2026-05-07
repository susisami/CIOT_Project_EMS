#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Initializes/initialize.h" // IWYU pragma: keep
#include "../Interrupt/interrupt.h"
#include "../Config/config.h"




/* ENUMS */
    typedef enum {
        WAIT_FIRST_EDGE,
        COUNT_FIRST,
        COUNT_SECOND,
        CALIBRATED
    } calib_state_t;

    
/* FUNCTION PROTOTYPES */
    void motor_calibration(uint *steps_per_rotation, uint *opto_gap_steps);
    void motor_step(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int step_nr);
    void position_correction(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int calibration_correction_steps);
    void get_avg_steps(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[DIVIDE_ROTATION]);



/* FUNCTIONS */


    // CALIBRATE THE MOTOR
    /*
        1.  count average steps in two parts divided by the two opto edges
        
        2.  correct the dispenser wheel position 
                if (count_first_avg > count_second_avg) {reverse half of the count_second_avg}  

                else {continue clockwise for half of the count_first_avg}
    */
    void motor_calibration(uint *steps_per_rotation, uint *opto_gap_steps)
    {
        // ENABLE INTERRUPT
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

        // VARIABLES
        int count_first_avg = 0;
        int count_second_avg = 0;
        int correction_steps = 0;
        int results[DIVIDE_ROTATION];

        int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS] = { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1} };



        // GET AVERAGE STEPS
        get_avg_steps(mtr_steps_arr, results);

        count_first_avg = results[0];
        count_second_avg = results[1];

        *steps_per_rotation = count_first_avg + count_second_avg;


        // CORRECT THE POSITION
        if (count_first_avg < count_second_avg)
        {
            correction_steps = count_first_avg / 2; // continue half of the first step count
            *opto_gap_steps = count_first_avg;
        }
        else if (count_first_avg > count_second_avg)
        {
            correction_steps = count_second_avg / -2; // reverse half of the second step count
            *opto_gap_steps = count_second_avg;
        }
        
        position_correction(mtr_steps_arr, correction_steps);


        // DISABLE INTERRUPT
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
    }




// GET AVERAGE AMOUNT OF STEPS PER ROTATION
    void get_avg_steps(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[DIVIDE_ROTATION])
    {
        /*
            Divide full rotation into two segments/variables:
                "count_first"     (steps between   1st and 2nd   opto edge)    
                "count_second"    (steps between   2nd and 3rd   opto edge)

            Program flow: 
                1. Rotate until first falling or rising edge from opto:
                2. Counting steps into the  "count_first"  until the second edge
                3. Between 2nd and 3rd edges count the steps into the  "count_second"
                4. 3rd -> 4th to  "count_first"  again 
                5. Keep counting until  "rotations"  amount of rotations done

        */

        int count_first = 0; // steps between   1st and 2nd   opto edge
        int count_second = 0; // steps between   2nd and 3rd   opto edge
        int counter = CALIBRATION_ROTATION_TIMES; 
        bool dummy_opto_edge;
        calib_state_t state = WAIT_FIRST_EDGE;


        while (state != CALIBRATED)
        {    

            for (int step_nr = 0; step_nr < MTR_PHASE_AMOUNT; step_nr++)
            {
                motor_step(mtr_steps_arr, step_nr);

                bool change_state = queue_try_remove(&opto_queue, &dummy_opto_edge);


                // STATE MACHINE
                switch (state) {

                    // ROTATE UNTIL THE FIRST EDGE
                    case WAIT_FIRST_EDGE:
                        if (change_state) 
                        {
                            state = COUNT_FIRST;
                        }

                        break;
                    
                    // Count until the next edge (first section between opto edges)
                    case COUNT_FIRST:
                        if (change_state)
                        {
                            state = COUNT_SECOND;
                        }
                        else
                        {
                            count_first++;
                            state = COUNT_FIRST;
                        }
                        break;

                    // Count until the next edge (second section between opto edges) 
                    //      and return to COUNT_FIRST until counter==0 (= motor_calibrationD) 
                    case COUNT_SECOND:
                        if (change_state)
                        {
                            counter--; // counter amount of rotations for avg steps
                            if (counter > 0)
                            {
                                state = COUNT_FIRST;
                            }
                            else
                            {
                                state = CALIBRATED;
                            }
                        }
                        else
                        {
                            count_second++;
                            state = COUNT_SECOND;
                        }
                        
                        break;
                    
                    
                    case CALIBRATED:
                        break;
                }

                sleep_us(MTR_SLEEP_US);
            }
        }
        results[0] = (float)count_first / CALIBRATION_ROTATION_TIMES;
        results[1] = (float)count_second / CALIBRATION_ROTATION_TIMES;
    }


        
// MOTOR DRIVERS
    void motor_step(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int step_nr)
    {
        gpio_put(MTR_IN1, mtr_steps_arr[step_nr][0]);
        gpio_put(MTR_IN2, mtr_steps_arr[step_nr][1]);
        gpio_put(MTR_IN3, mtr_steps_arr[step_nr][2]);
        gpio_put(MTR_IN4, mtr_steps_arr[step_nr][3]);
    }


    void position_correction(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int calibration_correction_steps)
    {
        /*
            calibration_correction_steps
                Positive value -> drive motor clockwise
                Negative value -> drive motor anti-clockwise

            Correcting the position during calibration by driving the motor the amount of calibration_correction_steps
        */


        // ANTI-CLOCKWISE
        if (calibration_correction_steps < 0)
        {
            calibration_correction_steps *= -1;

            for (int steps_left = calibration_correction_steps; steps_left > 0; steps_left--) 
            {
                int step_nr = steps_left % 8;
                motor_step(mtr_steps_arr, step_nr);
                sleep_us(MTR_SLEEP_US);
            }
        }
        // CLOCKWISE
        else
        {
            for (int steps_left = 0; steps_left < calibration_correction_steps; steps_left++) 
            {
                int step_nr = steps_left % 8;
                motor_step(mtr_steps_arr, step_nr);
                sleep_us(MTR_SLEEP_US);
            }
        }
    }
