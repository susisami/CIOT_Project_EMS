/* LIBRARIES */
#include <stdlib.h>
#include "pico/stdlib.h" // IWYU pragma: keep
#include "pico/util/queue.h" // IWYU pragma: keep
#include "../Config/config.h"


/* ENUMS */
// RUN DIRECTIONS
typedef enum { CLOCKWISE, COUNTERCLOCKWISE } run_direction_t;


/* FUNCTIONS */

int stepper_motor_run(const uint direction)
{
    static const int driving_sequence[MTR_PHASE_AMOUNT][MTR_COILS] = {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };
    static uint carousel_speed = CAROUSEL_MIN_SPEED;
    static uint i = 0;

    if (direction == CLOCKWISE)
    {
        gpio_put(MTR_IN1, driving_sequence[i][0]);
        gpio_put(MTR_IN2, driving_sequence[i][1]);
        gpio_put(MTR_IN3, driving_sequence[i][2]);
        gpio_put(MTR_IN4, driving_sequence[i][3]);
    }

    else
    {
        gpio_put(MTR_IN1, driving_sequence[i][3]);
        gpio_put(MTR_IN2, driving_sequence[i][2]);
        gpio_put(MTR_IN3, driving_sequence[i][1]);
        gpio_put(MTR_IN4, driving_sequence[i][0]);
    }

    sleep_ms(carousel_speed);

    i++;

    if (i > 7)
    {
        i = 0;

        if (carousel_speed > CAROUSEL_MAX_SPEED) carousel_speed--;
    }

    return 0;
}

int run_motor(const uint steps_per_rev, int times)
{
    uint ttl_steps = 0;

    while (ttl_steps < times * (steps_per_rev / 8))
    {
        stepper_motor_run(CLOCKWISE);
        ttl_steps++;
    }

    return 0;
}
