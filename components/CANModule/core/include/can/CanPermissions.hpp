#pragma once 

#include "can_gen/CanMessageId.hpp"
#include "can_gen/CanModule.hpp"


/**
 * @brief Checks whether a module is allowed to transmit a CAN message.
 *
 * Uses the generated CAN registry to verify that the given module
 * is the owner of the specified message.
 *
 * @param module The module attempting to send the message.
 * @param msg The CAN message identifier.
 * @return True if the module is allowed to send the message, false otherwise.
 */
bool isAllowedToSend(CanModule module, CanMessageId msg);
