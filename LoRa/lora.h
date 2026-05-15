#ifndef LORA_H
#define LORA_H

/* LIBRARIES */
#include <stdbool.h>
#include "pico/stdlib.h" // IWYU pragma: keep


/* ENUMS */
// forward declaration
typedef struct SystemInformation sys_info_t;

// LORA STATES
typedef enum {
    LORA_UNINITIALIZED,
    LORA_TESTING,
    LORA_CONFIGURING,
    LORA_JOINING,
    LORA_READY,
    LORA_ERROR
} lora_state_t;

// EVENT TYPES
typedef enum {
    EVENT_BOOT,
    EVENT_PILL_DISPENSED,
    EVENT_PILL_NOT_DISPENSED,
    EVENT_DISPENSER_EMPTY,
    EVENT_POWER_LOSS_PRE_CALIB,
    EVENT_POWER_LOSS_CALIB,
    EVENT_POWER_LOSS_PRE_DISPENSE,
    EVENT_POWER_LOSS_DISPENSE_IDLE,
    EVENT_POWER_LOSS_DISPENSE_RUNNING,
    EVENT_RESET
} lora_event_t;


/* STRUCTURES */
// LORA MODULE STRUCTURE
typedef struct {
    lora_state_t state;
    bool joined;
    uint8_t retry_count;
    char deveui[32];
} lora_module_t;


/* FUNCTION DECLARATIONS */
// Initialize LoRa module (UART and structure)
void lora_init(lora_module_t *module);

// Initialize and join the LoRaWAN network
bool lora_connect(lora_module_t *module);

// Send an event message to LoRaWAN network
bool lora_send_event(lora_module_t *module, lora_event_t event, sys_info_t *systemVariables, const char *data);

// Get current state of LoRa module
lora_state_t lora_get_state(lora_module_t *module);

// Read a line from UART (handles \n, ignores \r)
bool lora_uart_read_line(char *buffer, int max_len, int timeout_ms);

// Send AT command and wait for response
bool lora_send_command(const char *command, char *response, int response_len, int timeout_ms);

// Convert event type to message string
const char *lora_event_to_string(lora_event_t event);

void lora_power_loss_event(lora_module_t *lora, sys_info_t *systemVariables);


#endif // LORA_H
