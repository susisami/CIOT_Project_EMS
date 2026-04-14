#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Macros/macros.h"
#include "../Initializes/initialize.h"


void calibrate(int *steps_per_rev, int *irq_pin, int steps[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT]);
void motor_step(int steps[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], int i);
void motor_run(int steps_amount, int n, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], bool init_run, int init_steps);
void gpio_callback(uint gpio, uint32_t events);



void motor_calibration(sys_info_t *systemVariables)
{
    int steps_per_rev = 0;
    int irq_pin = 0;


    // motor driving
    int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT] = { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1} };


    calibrate(&steps_per_rev, &irq_pin, mtr_steps_arr);


    systemVariables->steps_per_rev = steps_per_rev;
    systemVariables->isCalibrated = true;

}


void calibrate(int *steps_per_rev, int *irq_pin, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT])
{
    // go until first opto edge -> start counting steps
    // save steps between 1st and 2nd opto
    // save steps between 2nd and 3rd opto
    // if 1.-2. has more steps than 2.-3. then reverse half of 2.-3., otherwise continue into the same direction for half of the 1.-2.
    int count_first = 0;
    int count_second = 0;
    int correction_steps = 0;
    bool counting_first = false;
    bool counting_second = false;
    bool initialized = false;

    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    while (queue_try_remove(&event_queue, &irq_pin));

    while (!initialized)
    {
        for (int i = 0; i < 8; i++)
        {
            motor_step(mtr_steps_arr, i);

            // check queue after each step
            while (queue_try_remove(&event_queue, &irq_pin))
            {
                if (irq_pin == OPTO_FORK)
                {
                    if (counting_first)
                    {
                        counting_second = true;
                        counting_first = false;
                    }
                    else if (counting_second) 
                    {
                        initialized = true;
                        counting_second = false;
                    }
                    else
                    {
                        counting_first = true;
                    }
                }
            }

            // count the steps between
            if (counting_second)
            {
                count_second++;
            }
            else if (counting_first)
            {
                count_first++;
            }
            sleep_us(MTR_SLEEP_US);
        }
    }

    *steps_per_rev = count_first + count_second;

    if (count_first < count_second)
    {
        correction_steps = count_first / 2; // continue half of the first step count

    }
    else if (count_first > count_second)
    {
        correction_steps = count_second / -2; // reverse half of the second step count
    }
    
    motor_run(0, 0, mtr_steps_arr, true, correction_steps);

    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
}



void motor_step(int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], int i)
{
    gpio_put(MTR_IN1, mtr_steps_arr[i][0]);
    gpio_put(MTR_IN2, mtr_steps_arr[i][1]);
    gpio_put(MTR_IN3, mtr_steps_arr[i][2]);
    gpio_put(MTR_IN4, mtr_steps_arr[i][3]);
}


void motor_run(int steps_amount, int n, int mtr_steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], bool init_run, int init_steps)
{
    int steps_total = 0;

    if (init_run) // init_motor defines the needed steps to correct the wheel position
    {
        steps_total = init_steps;
    }
    else
    {
        steps_total = steps_amount / 8 * n;
    }

    if (steps_total < 0)
    {
        steps_total *= -1;

        for (int steps_left = steps_total; steps_left > 0; steps_left--) // correct the initial position to the anti-clockwise direction
        {
            int step_nr = steps_left % 8;
            motor_step(mtr_steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }
    else
    {
        for (int steps_left = 0; steps_left < steps_total; steps_left++) // correct the initial position to the clockwise direction
        {
            int step_nr = steps_left % 8;
            motor_step(mtr_steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }
}
