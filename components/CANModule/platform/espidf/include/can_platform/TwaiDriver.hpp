#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

#include "can/ICanDriver.hpp"

extern "C" {
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
}

/**
 * @brief ESP-IDF TWAI implementation of the portable CAN driver interface.
 *
 * TX timeout:
 *   txTimeoutMs controls how long transmit() blocks waiting for the TX done
 *   callback. Set this to match your task's deadline. If the callback does
 *   not fire within the timeout (e.g. bus-off, hardware fault), transmit()
 *   returns false and the calling task is unblocked immediately.
 *
 * Bus-off recovery:
 *   In ESP-IDF v6.0, bus-off is detected via the on_state_change callback.
 *   When TWAI_ERROR_BUS_OFF state is entered, busOffCount_ is incremented
 *   and twai_node_recover() is called automatically.
 *   Use getBusOffCount() to monitor bus health from your application.
 */
class TwaiDriver : public ICanDriver {
public:
    /**
     * @param nodeHandle    TWAI node handle from twai_node_create().
     * @param rxQueueDepth  Depth of the RX frame queue (default: 16).
     * @param txTimeoutMs   Max time to wait for TX done callback in ms (default: 50).
     */
    explicit TwaiDriver(twai_node_handle_t nodeHandle,
                        size_t rxQueueDepth = 16,
                        uint32_t txTimeoutMs = 50);
    ~TwaiDriver();

    /**
     * @brief Registers TWAI callbacks and prepares the driver for use.
     *
     * Must be called before twai_node_enable().
     *
     * @return true if initialization succeeded, false otherwise.
     */
    bool init();

    bool transmit(const CanFrame& frame) override;
    bool receive(CanFrame& frame, uint32_t timeoutMs) override;

    /**
     * @brief Number of frames dropped due to RX queue overflow.
     */
    uint32_t getDroppedCount() const;

    /**
     * @brief Number of RX errors from TWAI (failed frame extraction).
     */
    uint32_t getErrorCount() const;

    /**
     * @brief Number of times the node has entered bus-off state.
     *
     * A non-zero value indicates CAN bus health issues. Monitor this
     * from your application for fault logging and diagnostics.
     * The driver attempts automatic recovery on bus-off.
     */
    uint32_t getBusOffCount() const;

private:
    static bool onRxDone(twai_node_handle_t handle,
                         const twai_rx_done_event_data_t* eventData,
                         void* userCtx);

    static bool onTxDone(twai_node_handle_t handle,
                         const twai_tx_done_event_data_t* eventData,
                         void* userCtx);

    static bool onStateChange(twai_node_handle_t handle,
                              const twai_state_change_event_data_t* eventData,
                              void* userCtx);

    twai_node_handle_t    nodeHandle_;
    QueueHandle_t         rxQueue_;
    SemaphoreHandle_t     txDoneSem_;
    uint32_t              txTimeoutMs_;
    std::atomic<uint32_t> rxDroppedCount_{0};
    std::atomic<uint32_t> rxErrorCount_{0};
    std::atomic<uint32_t> busOffCount_{0};
};