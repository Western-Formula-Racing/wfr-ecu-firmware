#include "can_platform/TwaiStackFactory.hpp"
#include "can_platform/EspClock.hpp"

#include <memory>

#include "can/CanStack.hpp"
#include "can/ICanDriver.hpp"
#include "can/ICanMutex.hpp"
#include "can_platform/TwaiDriver.hpp"
#include "FreeRtosMutex.hpp"

extern "C" {
#include "esp_err.h"
#include "esp_twai_onchip.h"
#include "hal/gpio_types.h"
}

namespace {
bool enableTwaiNode(void* nativeHandle) {
    if (nativeHandle == nullptr) {
        return false;
    }

    auto nodeHandle = static_cast<twai_node_handle_t>(nativeHandle);
    return twai_node_enable(nodeHandle) == ESP_OK;
}

bool disableTwaiNode(void* nativeHandle) {
    if (nativeHandle == nullptr) {
        return false;
    }

    auto nodeHandle = static_cast<twai_node_handle_t>(nativeHandle);
    const esp_err_t err = twai_node_disable(nodeHandle);

    return (err == ESP_OK) || (err == ESP_ERR_INVALID_STATE);
}

void cleanupTwaiNode(void* nativeHandle) {
    if (nativeHandle == nullptr) {
        return;
    }

    auto nodeHandle = static_cast<twai_node_handle_t>(nativeHandle);

    (void)twai_node_disable(nodeHandle);
    (void)twai_node_delete(nodeHandle);
}
}  // namespace

CanStack createEspTwaiStack(const CanStackConfig& config) {
    if (config.platform != CanPlatform::EspTwai) {
        return CanStack{};
    }

    if (config.txPin < 0 || config.rxPin < 0) {
        return CanStack{};
    }

    twai_onchip_node_config_t nodeConfig = {};
    nodeConfig.io_cfg.tx = static_cast<gpio_num_t>(config.txPin);
    nodeConfig.io_cfg.rx = static_cast<gpio_num_t>(config.rxPin);
    nodeConfig.bit_timing.bitrate = config.bitrate;

    nodeConfig.tx_queue_depth = config.espidf.txQueueDepth;
    nodeConfig.fail_retry_cnt = config.espidf.failRetryCount;
    nodeConfig.intr_priority  = config.espidf.intrPriority;

    nodeConfig.flags.enable_self_test   = config.espidf.enableSelfTest   ? 1 : 0;
    nodeConfig.flags.enable_loopback    = config.espidf.enableLoopback    ? 1 : 0;
    nodeConfig.flags.enable_listen_only = config.espidf.enableListenOnly  ? 1 : 0;
    nodeConfig.flags.no_receive_rtr     = config.espidf.noReceiveRtr      ? 1 : 0;

    if (config.espidf.busOffIndicatorPin >= 0) {
        nodeConfig.io_cfg.bus_off_indicator =
            static_cast<gpio_num_t>(config.espidf.busOffIndicatorPin);
    }

    if (config.espidf.quantaClockOutPin >= 0) {
        nodeConfig.io_cfg.quanta_clk_out =
            static_cast<gpio_num_t>(config.espidf.quantaClockOutPin);
    }

    twai_node_handle_t nodeHandle = nullptr;
    if (twai_new_node_onchip(&nodeConfig, &nodeHandle) != ESP_OK) {
        return CanStack{};
    }

    auto driver = std::make_unique<TwaiDriver>(nodeHandle, config.rxQueueDepth);
    if (!driver) {
        cleanupTwaiNode(nodeHandle);
        return CanStack{};
    }

    auto clock = std::make_unique<EspClock>();
    if (!clock) {
        cleanupTwaiNode(nodeHandle);
        return CanStack{};
    }

    auto mutex = std::make_unique<FreeRtosMutex>();
    if (!mutex) {
        cleanupTwaiNode(nodeHandle);
        return CanStack{};
    }

    auto can = std::make_unique<CanInterface>(*driver, *mutex, *clock, config.module);
    if (!can) {
        cleanupTwaiNode(nodeHandle);
        return CanStack{};
    }

    std::unique_ptr<ICanDriver> driverPtr(std::move(driver));
    std::unique_ptr<ICanMutex>  mutexPtr(std::move(mutex));
    std::unique_ptr<IClock>     clockPtr(std::move(clock));
    return CanStack(
        config,
        std::move(driverPtr),
        std::move(mutexPtr),
        std::move(clockPtr),
        std::move(can),
        nodeHandle,
        &enableTwaiNode,
        &disableTwaiNode,
        &cleanupTwaiNode
    );
}