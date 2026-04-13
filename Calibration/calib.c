#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Initializes/initialize.h" 
#include "../Macros/macros.h"


#define QUEUE_SIZE 10
#define MTR_INPUT_AMOUNT 4
#define MTR_PHASE_AMOUNT 8
#define MTR_SLEEP_US 1000

queue_t opto_queue;

void calibrate_motor(int *steps_per_rev, int *opto_fork_value, int steps[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT]);
void motor_step(int steps[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], int i);
void motor_run(int steps_amount, int n, int steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], bool init_run, int init_steps);
void gpio_callback(uint gpio, uint32_t events);


/*

*/


int motor_calibration(void)
{
    int steps_per_rev = 0;
    int opto_fork_value = 0;

    stdio_init_all();
    gpio_init_all();
    queue_init(&opto_queue, sizeof(int), QUEUE_SIZE);

    // Enable interrupt
    gpio_set_irq_enabled_with_callback(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // motor driving
    int steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT] = { {1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,1,1}, {0,0,0,1}, {1,0,0,1} };


    calibrate_motor(&steps_per_rev, &opto_fork_value, steps_arr);

    return steps_per_rev;
}


void calibrate_motor(int *steps_per_rev, int *opto_fork_value, int steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT])
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
    while (queue_try_remove(&opto_queue, &opto_fork_value) == true);

    while (!initialized)
    {
        for (int i = 0; i < 8; i++)
        {
            motor_step(steps_arr, i);

            // check queue after each step
            while (queue_try_remove(&opto_queue, &opto_fork_value) == true)
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
    
    motor_run(0, 0, steps_arr, true, correction_steps);


    gpio_set_irq_enabled(OPTO_FORK, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, false);
}



void motor_step(int steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], int i)
{
    gpio_put(MTR_IN1, steps_arr[i][0]);
    gpio_put(MTR_IN2, steps_arr[i][1]);
    gpio_put(MTR_IN3, steps_arr[i][2]);
    gpio_put(MTR_IN4, steps_arr[i][3]);
}


void motor_run(int steps_amount, int n, int steps_arr[MTR_PHASE_AMOUNT][MTR_INPUT_AMOUNT], bool init_run, int init_steps)
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
            motor_step(steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }
    else
    {
        for (int steps_left = 0; steps_left < steps_total; steps_left++) // correct the initial position to the clockwise direction
        {
            int step_nr = steps_left % 8;
            motor_step(steps_arr, step_nr);
            sleep_us(MTR_SLEEP_US);
        }
    }
}


void gpio_callback(uint gpio, uint32_t events)
{
    int value = 1;
    queue_try_add(&opto_queue, &value);
}