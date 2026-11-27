#pragma once

#include <cstdint>

#include "can/CanPlatform.hpp"
#include "can_gen/CanModule.hpp"

/**
 * @brief ESP-IDF TWAI-specific options.
 *
 * These use plain integer types so the config stays portable and
 * does not pull ESP-IDF hardware types into core headers.
 */
struct EspTwaiOptions {
    uint32_t txQueueDepth = 5;
    uint32_t failRetryCount = 1;

    int busOffIndicatorPin = -1;
    int quantaClockOutPin = -1;

    int intrPriority = 0;

    // Misc flags — map directly to twai_onchip_node_config_t::flags
    bool enableSelfTest   = false; ///< TX does not require acknowledgment
    bool enableLoopback   = false; ///< Controller receives its own frames
    bool enableListenOnly = false; ///< No TX or ACK, monitor only
    bool noReceiveRtr     = false; ///< Don't receive remote frames
};

/**
 * @brief Configuration used to construct a CAN stack.
 * 
 * This is a hardware-agnostic configuration struct that can be 
 * used to create a CAN stack on any supported platform.
 */
struct CanStackConfig {
    CanPlatform platform = CanPlatform::EspTwai;
    CanModule module = CanModule::MODULE_UNKNOWN;

    int txPin = -1;
    int rxPin = -1;

    uint32_t bitrate = 500000;
    uint32_t rxQueueDepth = 32;

    EspTwaiOptions espidf{};
};