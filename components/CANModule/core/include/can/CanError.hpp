#pragma once

#include <cstdint>

/**
 * @brief Error codes returned by CanInterface send operations.
 *
 * Allows callers to distinguish between different failure modes
 * for diagnostics, fault logging, and state machine decisions.
 */
enum class CanError : uint8_t {
    Ok,              ///< Operation succeeded.
    UnknownMessage,  ///< Message ID not found in registry.
    NotAllowed,      ///< This module is not the owner of the message.
    InvalidFrame,    ///< Frame ID, DLC, or extended flag does not match registry.
    DriverError,     ///< Driver rejected the frame (e.g. bus-off, queue full).
};