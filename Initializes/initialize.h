#ifndef INITIALIZE_H
#define INITIALIZE_H

#include "pico/util/queue.h"


// SYSTEM STRUCTURE
typedef struct SystemInformation
{
    bool isCalibrated; 
    uint avg_steps;
    uint steps_per_rev; // total steps per full cycle (approx. 4096)
    uint dispenser_position; // position from 0 to steps_per_rev
    bool pressed;
    bool led_state;

} sys_info_t;


// INTERRUPT QUEUE 
extern queue_t event_queue;
// TIMESTAMP FOR DEBOUNCE 
extern absolute_time_t last_press_time;


// INTERRUPT CALLBACK
void interrupt_callback(uint gpio, uint32_t events);
void init_gpio_all(void);

#endif //INITIALIZE_H