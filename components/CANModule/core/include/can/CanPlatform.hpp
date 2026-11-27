#pragma once

#include <cstdint>

/**
 * @brief Supported platform backends for CAN stack creation.
 *
 * Only EspIdfTwai is implemented right now.
 * The others exist so the public API is future-ready.
 */
enum class CanPlatform : uint8_t {
    EspTwai = 0,
    Mock
};