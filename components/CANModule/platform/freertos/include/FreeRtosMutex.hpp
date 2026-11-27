#pragma once

#include "can/ICanMutex.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief FreeRTOS implementation of ICanMutex.
 *
 * Uses a standard FreeRTOS mutex semaphore. Compatible with any MCU
 * running FreeRTOS — ESP32, STM32, RP2040, etc.
 *
 * Must be constructed before the RTOS scheduler starts, or from a task
 * context after the scheduler has started. Asserts if mutex creation fails.
 */
class FreeRtosMutex : public ICanMutex {
public:
    FreeRtosMutex();
    ~FreeRtosMutex() override;

    /**
     * @brief Acquire the mutex, blocking indefinitely until available.
     */
    void lock() override;

    /**
     * @brief Release the mutex.
     */
    void unlock() override;

private:
    SemaphoreHandle_t handle_;
};