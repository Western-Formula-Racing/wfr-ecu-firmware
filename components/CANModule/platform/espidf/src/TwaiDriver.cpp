#include "can_platform/TwaiDriver.hpp"

#include <algorithm>
#include <atomic>
#include <cstring>

extern "C" {
#include "esp_err.h"
#include "esp_log.h"
}

namespace {
constexpr const char* TAG = "TwaiDriver";
}

TwaiDriver::TwaiDriver(twai_node_handle_t nodeHandle, size_t rxQueueDepth, uint32_t txTimeoutMs)
    : nodeHandle_(nodeHandle),
      rxQueue_(xQueueCreate(
          static_cast<UBaseType_t>(rxQueueDepth > 0 ? rxQueueDepth : 1),
          sizeof(CanFrame))),
      txDoneSem_(xSemaphoreCreateBinary()),
      txTimeoutMs_(txTimeoutMs) {}

TwaiDriver::~TwaiDriver() {
    if (rxQueue_ != nullptr) {
        vQueueDelete(rxQueue_);
        rxQueue_ = nullptr;
    }

    if (txDoneSem_ != nullptr) {
        vSemaphoreDelete(txDoneSem_);
        txDoneSem_ = nullptr;
    }
}

bool TwaiDriver::init() {
    if (nodeHandle_ == nullptr) {
        return false;
    }

    if (rxQueue_ == nullptr) {
        return false;
    }

    if (txDoneSem_ == nullptr) {
        return false;
    }

    twai_event_callbacks_t callbacks = {};
    callbacks.on_rx_done     = &TwaiDriver::onRxDone;
    callbacks.on_tx_done     = &TwaiDriver::onTxDone;
    callbacks.on_state_change = &TwaiDriver::onStateChange;

    esp_err_t err = twai_node_register_event_callbacks(
        nodeHandle_,
        &callbacks,
        this
    );

    return err == ESP_OK;
}

bool TwaiDriver::transmit(const CanFrame& frame) {
    if (nodeHandle_ == nullptr) {
        ESP_LOGE(TAG, "transmit failed: node handle is null");
        return false;
    }

    if (txDoneSem_ == nullptr) {
        ESP_LOGE(TAG, "transmit failed: txDoneSem is null");
        return false;
    }

    if (frame.dlc > 8U) {
        ESP_LOGE(TAG, "transmit failed: invalid DLC %u", static_cast<unsigned>(frame.dlc));
        return false;
    }

    // Clear any stale TX-done signal before starting a new transmit.
    while (xSemaphoreTake(txDoneSem_, 0) == pdTRUE) {
    }

    uint8_t txBuffer[8] = {0};
    std::memcpy(txBuffer, frame.data, frame.dlc);

    twai_frame_t txFrame = {};
    txFrame.header.id = frame.id;
    txFrame.header.ide = frame.extended;
    txFrame.header.rtr = false;
    txFrame.header.fdf = false;
    txFrame.buffer = txBuffer;
    txFrame.buffer_len = frame.dlc;

    esp_err_t err = twai_node_transmit(nodeHandle_, &txFrame, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "transmit failed: twai_node_transmit error %d", static_cast<int>(err));
        return false;
    }

    if (xSemaphoreTake(txDoneSem_, pdMS_TO_TICKS(txTimeoutMs_)) != pdTRUE) {
        ESP_LOGE(TAG, "transmit failed: no TX done callback within %lums", txTimeoutMs_);
        return false;
    }

    return true;
}

bool TwaiDriver::receive(CanFrame& frame, uint32_t timeoutMs) {
    if (rxQueue_ == nullptr) {
        return false;
    }

    TickType_t ticks = 0;
    if (timeoutMs == 0U) {
        ticks = 0;
    } else if (timeoutMs == static_cast<uint32_t>(portMAX_DELAY)) {
        ticks = portMAX_DELAY;
    } else {
        ticks = pdMS_TO_TICKS(timeoutMs);
    }

    return xQueueReceive(rxQueue_, &frame, ticks) == pdTRUE;
}

uint32_t TwaiDriver::getDroppedCount() const {
    return rxDroppedCount_.load(std::memory_order_relaxed);
}

uint32_t TwaiDriver::getErrorCount() const {
    return rxErrorCount_.load(std::memory_order_relaxed);
}

uint32_t TwaiDriver::getBusOffCount() const {
    return busOffCount_.load(std::memory_order_relaxed);
}

bool TwaiDriver::onRxDone(twai_node_handle_t handle,
                          const twai_rx_done_event_data_t*,
                          void* userCtx) {
    if (handle == nullptr || userCtx == nullptr) {
        return false;
    }

    auto* self = static_cast<TwaiDriver*>(userCtx);
    if (self->rxQueue_ == nullptr) {
        return false;
    }

    uint8_t rxBuffer[8] = {0};

    twai_frame_t rxFrame = {};
    rxFrame.buffer = rxBuffer;
    rxFrame.buffer_len = sizeof(rxBuffer);

    if (twai_node_receive_from_isr(handle, &rxFrame) != ESP_OK) {
        self->rxErrorCount_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    CanFrame frame = {};
    frame.id = rxFrame.header.id;
    frame.extended = rxFrame.header.ide;
    frame.dlc = static_cast<uint8_t>(
        std::min<size_t>(rxFrame.buffer_len, sizeof(frame.data))
    );

    std::memset(frame.data, 0, sizeof(frame.data));
    std::memcpy(frame.data, rxBuffer, frame.dlc);

    BaseType_t higherPriorityTaskWoken = pdFALSE;
    const BaseType_t result =
        xQueueSendFromISR(self->rxQueue_, &frame, &higherPriorityTaskWoken);

    if (result != pdTRUE) {
        self->rxDroppedCount_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }

    return true;
}

bool TwaiDriver::onTxDone(twai_node_handle_t,
                          const twai_tx_done_event_data_t*,
                          void* userCtx) {
    if (userCtx == nullptr) {
        return false;
    }

    auto* self = static_cast<TwaiDriver*>(userCtx);
    if (self->txDoneSem_ == nullptr) {
        return false;
    }

    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if (xSemaphoreGiveFromISR(self->txDoneSem_, &higherPriorityTaskWoken) != pdTRUE) {
        return false;
    }

    if (higherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }

    return true;
}

bool TwaiDriver::onStateChange(twai_node_handle_t handle,
                               const twai_state_change_event_data_t* eventData,
                               void* userCtx) {
    if (handle == nullptr || eventData == nullptr || userCtx == nullptr) {
        return false;
    }

    auto* self = static_cast<TwaiDriver*>(userCtx);

    if (eventData->new_sta == TWAI_ERROR_BUS_OFF) {
        self->busOffCount_.fetch_add(1, std::memory_order_relaxed);
        ESP_LOGE(TAG, "Bus-off detected (count: %lu) — attempting recovery",
                 self->busOffCount_.load(std::memory_order_relaxed));

        // twai_node_recover() re-enters error-active state after the
        // required 128 recessive bits sequence on the bus.
        const esp_err_t err = twai_node_recover(handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Bus-off recovery failed: %d", static_cast<int>(err));
            return false;
        }
    }

    return true;
}