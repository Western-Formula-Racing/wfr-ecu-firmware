// EspClock.hpp
#pragma once

#include "can/IClock.hpp"

extern "C" {
#include "esp_timer.h"
}

/**
 * @brief ESP-IDF implementation of IClock.
 *
 * Uses esp_timer_get_time() which returns microseconds since boot.
 * Converted to milliseconds on read.
 */
class EspClock : public IClock {
public:
    uint32_t nowMs() const override;
};