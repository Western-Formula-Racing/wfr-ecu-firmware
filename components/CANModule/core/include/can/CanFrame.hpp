#pragma once 

#include <cstdint>

/**************************************************
 * @file CanFrame.hpp
 * @brief Defines the CanFrame struct which represents a CAN message frame.
 *
 * This struct encapsulates all the necessary information about a CAN message,
 * including its ID, data length, payload, and whether it's an extended frame.
 * It serves as the fundamental unit of communication on the CAN bus.
 **************************************************/

struct CanFrame {
    uint32_t id = 0;
    uint8_t dlc = 0;
    bool extended = false;
    uint8_t data[8] = {0};
};