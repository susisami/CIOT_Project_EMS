#ifndef INTERRRUPT_H
#define INTERRRUPT_H

/*  LIBRARIES */
#include "pico/util/queue.h"


/* QUEUE DECLARATIONS */
// INTERRUPT QUEUE FOR ROT SW (GP12)
extern queue_t button_queue;

// INTERRUPT QUEUE FOR OPTO FORK (GP28)
extern queue_t opto_queue;

//INTERRUPT QUEUE FOR PIEZO SENSOR (GP27)
extern queue_t piezo_queue;


/* FUNCTION DECLARATIONS */
// INTERRUPT CALLBACK FOR ROT SW & OPTO FORK
void interrupt_callback(uint gpio, uint32_t events);


#endif //INTERRUPT_H
