#pragma once

/**
 * @brief Abstract mutex interface for RTOS synchronization.
 *
 * Implemented by platform-specific RTOS layers (e.g. FreeRTOS, Zephyr).
 * Injected into CanInterface to keep the core layer platform-agnostic.
 *
 * Implementations must be non-recursive mutexes suitable for use
 * from task context. ISR-context locking is not supported.
 */
class ICanMutex {
public:
    virtual ~ICanMutex() = default;

    /**
     * @brief Acquire the mutex, blocking indefinitely until available.
     */
    virtual void lock() = 0;

    /**
     * @brief Release the mutex.
     */
    virtual void unlock() = 0;
};