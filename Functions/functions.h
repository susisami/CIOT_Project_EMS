#ifndef FUNCTIONS_H
#define FUNCTIONS_H
/*  LIBRARIES */
#include "pico/util/queue.h"


/* QUEUES */

    // INTERRUPT QUEUE FOR ROT SW (GP12)
    extern queue_t button_queue;

    // INTERRUPT QUEUE FOR OPTO FORK (GP28)
    extern queue_t opto_queue;

    //INTERRUPT QUEUE FOR PIEZO SENSOR (GP27)
    extern queue_t pills_queue;


/* GLOBAL VARIABLES */

    // TIMESTAMP FOR DEBOUNCE
    extern absolute_time_t last_press_time;


/* FUNCTION DECLARATIONS */

    // INTERRUPT CALLBACK FOR ROT SW & OPTO FORK
    void interrupt_callback(uint gpio, uint32_t events);


#endif //FUNCTIONS_H