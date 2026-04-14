#ifndef INITIALIZE_H
#define INITIALIZE_H

#include "pico/util/queue.h"


// SYSTEM STRUCTURE
typedef struct SystemInformation
{
    bool isCalibrated; 

    uint avg_steps;
    // total steps per full cycle (approx. 4096)
    uint steps_per_rev; 
    // position from 0 to steps_per_rev (calibrated=0, increasing clockwise)
    uint dispenser_position; 
    // button pressed
    bool button_pressed;
    //
    bool led_on;

} sys_info_t;


// INTERRUPT QUEUE 
extern queue_t event_queue;
// TIMESTAMP FOR DEBOUNCE 
extern absolute_time_t last_press_time;


// INTERRUPT CALLBACK
void interrupt_callback(uint gpio, uint32_t events);
// init GPIOs
void init_gpio_all(void);
// init systemVariables
void init_sys_variables(sys_info_t *systemVariables);


#endif //INITIALIZE_H