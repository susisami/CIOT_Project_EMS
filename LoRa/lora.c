#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lora.h"
#include "hardware/uart.h"
#include "../.env/lora_app_key.h"
#include "../Initializes/initialize.h" // IWYU pragma: keep
#include "../Config/config.h"


//Forget everything the module said
static void uart_flush(void)
{
    while (uart_is_readable(LORA_UART_ID))
    {
        uart_getc(LORA_UART_ID);
    }
}

// get Devui for your lora grove module
void lora_process_deveui(char* input, char* output)
{
    int j = 0;

    for (int i = 0; input[i] != '\0'; i++)
    {
        if (input[i] != ':')
        {
            output[j++] = (char)tolower((unsigned char)input[i]);
        }
    }

    output[j] = '\0';
}

// Validate single response line
static bool lora_check_response(const char* command, const char* response)
{
    if (!response || response[0] == '\0')
        return false;

    if (strcmp(command, "AT") == 0)
        return strstr(response, "+AT:") != NULL;

    if (strstr(command, "AT+VER"))
        return strstr(response, "+VER:") != NULL;

    if (strstr(command, "AT+ID"))
        return strstr(response, "+ID:") != NULL;

    if (strstr(command, "AT+MODE"))
        return strstr(response, "+MODE:") != NULL;

    if (strstr(command, "AT+KEY"))
        return strstr(response, "+KEY:") != NULL;

    if (strstr(command, "AT+CLASS"))
        return strstr(response, "+CLASS:") != NULL;

    if (strstr(command, "AT+PORT"))
        return strstr(response, "+PORT") != NULL;

    if (strstr(command, "AT+DR"))
        return strstr(response, "+DR") != NULL;

    if (strstr(command, "AT+JOIN"))
        return strstr(response, "+JOIN") != NULL;

    return strstr(response, "ERROR") == NULL;
}


//read uart
bool lora_uart_read_line(char* buffer, int max_len, int timeout_ms)
{
    int pos = 0;
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);

    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
    {
        if (uart_is_readable(LORA_UART_ID))
        {
            char c = uart_getc(LORA_UART_ID);

            if (c != '\r')
            {
                if (c == '\n')
                {
                    buffer[pos] = '\0';
                    return true;
                }

                if (pos < max_len - 1)
                {
                    buffer[pos++] = c;
                }
            }
        }
    }
    buffer[pos] = '\0';
    return false;
}

//send command to lora module
bool lora_send_command(const char* command, char* response, int response_len, int timeout_ms)
{
    memset(response, 0, response_len);
    uart_flush();

    printf("[LoRa TX] %s\n", command);

    uart_write_blocking(LORA_UART_ID, (const uint8_t*)command, strlen(command));
    uart_write_blocking(LORA_UART_ID, (const uint8_t*)"\r\n", strlen("\r\n"));

    if (!lora_uart_read_line(response, response_len, timeout_ms))
    {
        printf("[LoRa RX] <timeout>\n");
        return false;
    }

    printf("[LoRa RX] %s\n", response);

    return lora_check_response(command, response);
}


void lora_init(lora_module_t* module)
{
    uart_init(LORA_UART_ID, LORA_BAUD_RATE);
    gpio_set_function(LORA_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(LORA_RX_PIN, GPIO_FUNC_UART);

    module->state = LORA_UNINITIALIZED;
    module->joined = false;
    module->retry_count = 0;
    module->deveui[0] = '\0';

    // 1 sec delay for LoRa module to boot
    sleep_ms(LED_BLINK_SLOW_MS);
}


//

static bool lora_step_join(char* buf)
{
    absolute_time_t deadline = make_timeout_time_ms(LORA_JOIN_TIMEOUT_MS);

    while (absolute_time_diff_us(get_absolute_time(), deadline) > 0)
    {
        if (lora_uart_read_line(buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        {
            printf("[LoRa RX] %s\n", buf);

            if (strstr(buf, "+JOIN: Done"))
                return true;

            if (strstr(buf, "+JOIN: Joined already"))
                return true;

            if (strstr(buf, "Join failed"))
                return false;
        }
    }

    return false;
}

bool lora_connect(lora_module_t* module)
{
    char buf[LORA_BUFFER_SIZE]; //for rx
    char cmd[LORA_COMMAND_SIZE]; //for tx

    uart_flush();

    module->state = LORA_TESTING;
    printf("[LoRa] Testing module...\n");

    // Retry AT command in case module is still booting
    bool at_success = false;
    for (int i = 0; i < 3; i++)
    {
        if (lora_send_command("AT", buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        {
            at_success = true;
            break;
        }
        printf("[LoRa] AT retry %d/3\n", i + 1);
        sleep_ms(1000);
    }

    if (!at_success)
    {
        module->state = LORA_ERROR;
        return false;
    }

    // version and devui are not necessary for project
    lora_send_command("AT+VER", buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS);
    printf("[LoRa] Version: %s\n", buf);

    if (lora_send_command("AT+ID=DevEui", buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
    {
        char* comma = strchr(buf, ',');

        if (comma != NULL)
        {
            comma++; // move past ','

            while (*comma == ' ') comma++; // skip spaces

            lora_process_deveui(comma, module->deveui);

            printf("[LoRa] DevEui: %s\n", module->deveui);
        }
        else
        {
            printf("[LoRa] Invalid DevEui format: %s\n", buf);
            return false;
        }
    }

    module->state = LORA_CONFIGURING;
    printf("[LoRa] Configuring...\n");

    if (!lora_send_command("AT+MODE=LWOTAA", buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        return false;

    snprintf(cmd, sizeof(cmd), "AT+KEY=APPKEY,\"%s\"", LORA_APP_KEY);
    if (!lora_send_command(cmd, buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        return false;

    if (!lora_send_command("AT+CLASS=A", buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        return false;

    snprintf(cmd, sizeof(cmd), "AT+PORT=%d", LORA_PORT);
    if (!lora_send_command(cmd, buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        return false;

    snprintf(cmd, sizeof(cmd), "AT+DR=%d", LORA_DATA_RATE);
    if (!lora_send_command(cmd, buf, LORA_BUFFER_SIZE, LORA_JOIN_TIMEOUT_MS))
        return false;

    module->state = LORA_JOINING;
    printf("[LoRa] Joining network...\n");

    bool joined = false;

    for (int i = 0; i < LORA_MAX_RETRY_ATTEMPTS; i++)
    {
        printf("[LoRa] Join attempt %d/%d\n", i + 1, LORA_MAX_RETRY_ATTEMPTS);

        uart_flush();

        uart_write_blocking(LORA_UART_ID, (const uint8_t*)"AT+JOIN\r\n", strlen("AT+JOIN\r\n"));

        if (lora_step_join(buf))
        {
            joined = true;
            break;
        }

        sleep_ms(LORA_JOIN_TIMEOUT_MS);
    }

    if (!joined)
    {
        module->state = LORA_ERROR;
        return false;
    }

    module->joined = true;
    module->state = LORA_READY;

    printf("[LoRa] Connected successfully!\n");
    return true;
}


bool lora_send_event(lora_module_t* module, lora_event_t event, const char* data)
{
    if (module->state != LORA_READY || !module->joined)
        return false;

    char msg[LORA_MESSAGE_SIZE];
    char cmd[LORA_COMMAND_SIZE];
    char resp[LORA_BUFFER_SIZE];

    if (data)
        snprintf(msg, sizeof(msg), "%s: %s",
                 lora_event_to_string(event), data);
    else
        snprintf(msg, sizeof(msg), "%s",
                 lora_event_to_string(event));

    snprintf(cmd, sizeof(cmd), "AT+MSG=\"%s\"", msg);

    printf("[LoRa] Sending: %s\n", msg);

    for (int i = 0; i < LORA_MAX_RETRY_ATTEMPTS; i++)
    {
        if (!lora_send_command(cmd, resp,LORA_BUFFER_SIZE,LORA_MSG_SEND_TIMEOUT_MS))
        {
            return false;
        }

        if (strstr(resp, "busy") == NULL)
        {
            printf("[LoRa] Message sent" " \n");
            return true;
        }

        printf("[LoRa] Modem busy, retry %d/3\n", i + 1);

        sleep_ms(LORA_MSG_SEND_TIMEOUT_MS);
    }

    printf("[LoRa] Failed to send message\n");
    return false;
}

void lora_power_loss_event(lora_module_t* lora, sys_info_t* systemVariables)
{
    // Otherwise → power loss happened
    switch (systemVariables->program_state)
    {
    case PRE_CALIB:
        lora_send_event(lora, EVENT_POWER_LOSS_PRE_CALIB, NULL);
        break;

    case CALIB:
        lora_send_event(lora, EVENT_POWER_LOSS_CALIB, NULL);
        break;

    case PRE_DISPENSE:
        lora_send_event(lora, EVENT_POWER_LOSS_PRE_DISPENSE, NULL);
        break;

    case DISPENSE:
        if (systemVariables->isRunning)
        {
            lora_send_event(lora, EVENT_POWER_LOSS_DISPENSE_RUNNING, NULL);
        }
        else
        {
            lora_send_event(lora, EVENT_POWER_LOSS_DISPENSE_IDLE, NULL);
        }
        break;

    case RESET:
        lora_send_event(lora, EVENT_RESET, NULL);
        break;
    }
}

//state

lora_state_t lora_get_state(lora_module_t* module)
{
    return module->state;
}


//events

const char* lora_event_to_string(lora_event_t event)
{
    switch (event)
    {
    case EVENT_BOOT: return "BOOT";
    case EVENT_PILL_DISPENSED: return "PILL_DISPENSED";
    case EVENT_PILL_NOT_DISPENSED: return "PILL_NOT_DISPENSED";
    case EVENT_DISPENSER_EMPTY: return "DISPENSER_EMPTY";
    case EVENT_POWER_LOSS_PRE_CALIB: return "PWR_LOSS_PRE_CALIB";
    case EVENT_POWER_LOSS_CALIB: return "PWR_LOSS_CALIB";
    case EVENT_POWER_LOSS_PRE_DISPENSE: return "PWR_LOSS_PRE_DISP";
    case EVENT_POWER_LOSS_DISPENSE_IDLE: return "PWR_LOSS_DISP_IDLE";
    case EVENT_POWER_LOSS_DISPENSE_RUNNING: return "PWR_LOSS_DISP_RUN";
    case EVENT_RESET: return "RESET";
    default: return "UNKNOWN";
    }
}
