#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Initializes/initialize.h"
#include "../Functions/functions.h"
#include "../Macros/macros.h"


/* ENUMS */
    typedef enum {
        WAIT_FIRST_EDGE,
        COUNT_FIRST,
        COUNT_SECOND,
        CALIBRATED
    } calib_state_t;

    
/* FUNCTION PROTOTYPES */
    void calibrate(int *steps_per_rotation, int *opto_fork_value, int steps[MTR_PHASE_AMOUNT][MTR_COILS]);
    void motor_step(int steps[MTR_PHASE_AMOUNT][MTR_COILS], int step_nr);
    void position_correction(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int calibration_correction_steps);
    void get_avg_steps(int *opto_fork_value, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[DIVIDE_ROTATION]);



/* FUNCTIONS */

// MAIN CALIBRATION FUNCTION

    void motor_calibration(sys_info_t *systemVariables)
    {
        // INTERRUPT
        queue_init(&opto_queue, sizeof(int), QUEUE_SIZE);
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);


        // VARIABLES
        int steps_per_rotation = 0;
        int opto_fork_value = 0;
        int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS] = { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1} };
        

        // CALIBRATE MOTOR
        calibrate(&steps_per_rotation, &opto_fork_value, mtr_steps_arr);

        
        // DISABLE INTERRUPT
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);


        // VALUES TO MAIN PROGRAM !
        systemVariables->avg_steps = steps_per_rotation;
        systemVariables->isCalibrated = true;
    }


// GET AVERAGE AMOUNT OF STEPS PER ROTATION
    void get_avg_steps(int *opto_fork_value, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS], int results[DIVIDE_ROTATION])
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
        int counter = CALIBRATION_ROTATIONS; 
        calib_state_t state = WAIT_FIRST_EDGE;

        while (state != CALIBRATED)
        {    
            for (int step_nr = 0; step_nr < MTR_PHASE_AMOUNT; step_nr++)
            {
                motor_step(mtr_steps_arr, step_nr);

                // check opto fork queue after each motor step
                if (queue_try_remove(&opto_queue, &opto_fork_value)) 
                {
                    switch (state) {

                    case WAIT_FIRST_EDGE:
                        state = COUNT_FIRST;
                        break;
                    

                    case COUNT_FIRST:
                        state = COUNT_SECOND;
                        break;

                        
                    case COUNT_SECOND:
                        counter--;
                        if (counter > 0)
                        {
                            state = COUNT_FIRST;
                        }
                        else
                        {
                            state = CALIBRATED;
                        }
                        break;
                        
                        
                    case CALIBRATED:
                      break;
                    }
                }

                // count the steps between   1st and 2nd   &&   2nd and 3rd   OPTO EDGE
                if (state == COUNT_FIRST)
                {
                    count_first++;
                }
                else if (state == COUNT_SECOND)
                {
                    count_second++;
                }

                sleep_us(MTR_SLEEP_US);
            }
        }
        results[0] = (float)count_first / CALIBRATION_ROTATIONS;
        results[1] = (float)count_second / CALIBRATION_ROTATIONS;
    }


// CALIBRATE THE MOTOR
    void calibrate(int *steps_per_rotation, int *opto_fork_value, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_COILS])
    {
        /*
            1.  run get_avg_steps()
            
            2.  if (count_first_avg > count_second_Avg) {reverse half of the count_second_avg}  

                else {continue clockwise for half of the count_first_avg}
        */

        // VARIABLES
        int count_first_avg = 0;
        int count_second_avg = 0;
        int correction_steps = 0;
        int results[DIVIDE_ROTATION];

        // ENABLE INTERRUPT
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);

        // GET AVERAGE STEPS
        get_avg_steps(opto_fork_value, mtr_steps_arr, results);

        count_first_avg = results[0];
        count_second_avg = results[1];

        *steps_per_rotation = count_first_avg + count_second_avg;


        // CORRECT THE POSITION
        if (count_first_avg < count_second_avg)
        {
            correction_steps = count_first_avg / 2; // continue half of the first step count
        }
        else if (count_first_avg > count_second_avg)
        {
            correction_steps = count_second_avg / -2; // reverse half of the second step count
        }
        
        position_correction(mtr_steps_arr, correction_steps);


        // DISABLE INTERRUPT
        gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
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
