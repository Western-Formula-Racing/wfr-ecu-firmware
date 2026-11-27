#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Creates and starts the CAN stack (TWAI). Call once at startup, before creating the tasks.
bool can_init(void);

// RX task: the ONLY task that calls receiveAnyAndUpdate(); it fills the CanModule state cache.
void canRxTask(void* pvParameters);

// TX task: periodically sends the messages this module (MODULE_ECU) owns.
void canTxTask(void* pvParameters);

#ifdef __cplusplus
}
#endif
