#pragma once

#include <cstdint>
#include "can/CanFrame.hpp"

/**
 * @brief Interface for CAN bus drivers.
 *
 * Implementations must override transmit() and receive() to interface
 * with platform-specific CAN hardware (e.g., TWAI, MCP2515).
 */
class ICanDriver {
public:
    virtual ~ICanDriver() = default;

    /**
     * @brief Transmits a CAN frame.
     * @return True if transmission was successful, false otherwise.
     */
    virtual bool transmit(const CanFrame& frame) = 0;

    /**
     * @brief Receives a CAN frame.
     * @param frame Output frame.
     * @param timeoutMs Timeout in milliseconds.
     * @return True if a frame was received, false on timeout or error.
     */
    virtual bool receive(CanFrame& frame, uint32_t timeoutMs) = 0;

    /**
     * @brief Initializes the CAN driver.
     * @return True if initialization was successful, false otherwise.
     */
    virtual bool init() = 0;
};