#pragma once

#include <cstdint>

/**
 * @brief Abstract monotonic clock interface.
 *
 * Returns elapsed milliseconds since an arbitrary epoch (typically boot).
 * The value must be monotonically non-decreasing and safe to call from
 * task context.
 *
 * Implemented by platform-specific layers (e.g. EspClock for ESP-IDF).
 * Injected into CanInterface to keep the core layer platform-agnostic.
 */
class IClock {
public:
    virtual ~IClock() = default;

    /**
     * @brief Returns the current time in milliseconds.
     */
    virtual uint32_t nowMs() const = 0;
};