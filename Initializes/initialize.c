#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Macros/macros.h" 



void gpio_init_all(void)
{
    gpio_init(LED_2);
    gpio_set_dir(LED_2, GPIO_OUT);
    gpio_put(LED_2, true); 

    gpio_init(ROT_SW);
    gpio_set_dir(ROT_SW, GPIO_IN);
    gpio_pull_up(ROT_SW);
    
    gpio_init(OPTO_FORK);
    gpio_set_dir(OPTO_FORK, GPIO_IN);
    gpio_pull_up(OPTO_FORK);

    gpio_init(MTR_IN1);
    gpio_init(MTR_IN2);
    gpio_init(MTR_IN3);
    gpio_init(MTR_IN4);

    gpio_set_dir(MTR_IN1, GPIO_OUT);
    gpio_set_dir(MTR_IN2, GPIO_OUT);
    gpio_set_dir(MTR_IN3, GPIO_OUT);
    gpio_set_dir(MTR_IN4, GPIO_OUT);
}

