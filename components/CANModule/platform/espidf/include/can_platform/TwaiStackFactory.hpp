#pragma once

#include "can/CanStackConfig.hpp"

class CanStack;

/**
 * @brief Creates an ESP-IDF TWAI-backed CAN stack from generic config.
 */
CanStack createEspTwaiStack(const CanStackConfig& config);