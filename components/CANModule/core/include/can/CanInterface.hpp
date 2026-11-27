#pragma once

#include <cstdint>

#include "can/CanError.hpp"
#include "can/CanFrame.hpp"
#include "can/ICanDriver.hpp"
#include "can/ICanMutex.hpp"
#include "can/IClock.hpp"
#include "can_gen/CanMessageId.hpp"
#include "can_gen/CanModule.hpp"
#include "can_gen/CanMessageTraits.hpp"
#include "can_gen/CanStateCache.hpp"
#include "can_gen/CanStateCacheUpdate.hpp"


/**
 * @brief High-level CAN interface.
 *
 * Provides:
 * - validated raw frame send/receive
 * - typed message send/receive using generated message traits
 * - thread-safe access to CanStateCache via snapshot()
 *
 * Thread safety model:
 *   CanInterface owns the cache and protects it via an injected ICanMutex.
 *   The RX task calls receiveAnyAndUpdate() which holds the mutex only
 *   during the cache write. All other tasks call snapshot() to get a
 *   consistent point-in-time copy, then work on that copy freely.
 *
 * Platform independence:
 *   The mutex implementation is injected at construction (e.g. FreeRtosMutex).
 *   CanInterface itself has no RTOS dependency.
 */
class CanInterface {
public:
    /**
     * @param driver   Platform-specific CAN driver (e.g. TwaiDriver).
     * @param mutex    Platform-specific mutex (e.g. FreeRtosMutex).
     * @param module   The CAN module identity of this node.
     * @param clock    Platform-specific monotonic clock (e.g. EspClock).
     */
    CanInterface(ICanDriver& driver, ICanMutex& mutex, IClock& clock, CanModule module);

    /**
     * @brief Sends a validated raw CAN frame for a specific message ID.
     *
     * @return CanError::Ok on success, or a specific error code indicating
     *         why the send was rejected.
     */
    CanError sendRaw(CanMessageId msg, const CanFrame& frame);

    /**
     * @brief Receives a raw CAN frame from the driver.
     */
    bool receiveRaw(CanFrame& frame, uint32_t timeoutMs);

    /**
     * @brief Receives any CAN frame and updates the internal state cache.
     *
     * Decodes the received frame and writes it into the internal cache
     * under mutex protection. Intended to be called from the RX task only.
     */
    bool receiveAnyAndUpdate(uint32_t timeoutMs);

    /**
     * @brief Returns a consistent point-in-time copy of the state cache.
     *
     * Acquires the mutex, copies the entire cache, and releases immediately.
     * The returned copy is safe to read from any task without holding any lock.
     *
     * @return A value copy of the cache at this instant.
     */
    CanStateCache snapshot() const;

    /**
     * @brief Sends a typed CAN message.
     *
     * Uses the generated traits for the message type to encode the message,
     * resolve the correct message ID, and transmit the frame.
     *
     * @return CanError::Ok on success, or a specific error code on failure.
     */
    template <typename T>
    CanError send(const T& msg) {
        CanFrame frame{};
        if (!CanMessageTraits<T>::encode(msg, frame)) {
            return CanError::InvalidFrame;
        }
        return sendRaw(CanMessageTraits<T>::id, frame);
    }

private:
    ICanDriver&   driver_;
    ICanMutex&    mutex_;
    IClock&     clock_;
    CanModule     thisModule_;
    CanStateCache cache_;
};