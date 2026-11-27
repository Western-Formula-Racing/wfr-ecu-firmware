// EspClock.cpp
#include "can_platform/EspClock.hpp"

uint32_t EspClock::nowMs() const {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000U);
}