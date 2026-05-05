#include "pico/stdlib.h" // IWYU pragma: keep
#include "../Config/config.h"
#include "initialize.h"
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/binary_info.h" // IWYU pragma: keep


// EEPROM INIT
int init_i2c_instance(void)
{
    i2c_init(i2c0, I2C_FREQ);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));

    return 0;
}


// SYSTEM STRUCT
void init_sys_variables(sys_info_t *systemVariables)
{
    systemVariables->program_state = PRE_CALIB;
    systemVariables->avg_steps = 0;
    systemVariables->dispenser_position = 0;
    systemVariables->isRunning = false;

    systemVariables->isCalibrated = false;
    systemVariables->dispense_start_time = 0;
    systemVariables->dispensed_pills = 0;
    systemVariables->button_pressed = false;
}

// Init LoRa module
void init_lora(lora_module_t *lora_module)
{
    lora_init(lora_module);
    if (lora_connect(lora_module)) {
        lora_send_event(lora_module, EVENT_BOOT, NULL);
    }
    printf("Starting main loop\n");
}


// Init GPIOs
    void init_gpio_all(void)
    {
        // LEDS
        gpio_init(LED_1);
        gpio_init(LED_2);
        gpio_init(LED_3);
        gpio_set_dir(LED_1, GPIO_OUT);
        gpio_set_dir(LED_2, GPIO_OUT);
        gpio_set_dir(LED_3, GPIO_OUT);
        gpio_put(LED_1, false); 
        gpio_put(LED_2, true); 
        gpio_put(LED_3, false); 

        // ROT_SW
        gpio_init(ROT_SW);
        gpio_set_dir(ROT_SW, GPIO_IN);
        gpio_pull_up(ROT_SW);
        
        // OPTO_FORK
        gpio_init(OPTO_FORK);
        gpio_set_dir(OPTO_FORK, GPIO_IN);
        gpio_pull_up(OPTO_FORK);

        // PIEZO SENSOR
        gpio_init(PIEZO_SR);
        gpio_set_dir(PIEZO_SR, GPIO_IN);
        gpio_pull_up(PIEZO_SR);

        // MOTOR
        gpio_init(MTR_IN1);
        gpio_init(MTR_IN2);
        gpio_init(MTR_IN3);
        gpio_init(MTR_IN4);
        gpio_set_dir(MTR_IN1, GPIO_OUT);
        gpio_set_dir(MTR_IN2, GPIO_OUT);
        gpio_set_dir(MTR_IN3, GPIO_OUT);
        gpio_set_dir(MTR_IN4, GPIO_OUT);
    }

