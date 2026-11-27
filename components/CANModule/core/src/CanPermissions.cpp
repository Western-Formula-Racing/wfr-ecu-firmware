#include "can/CanPermissions.hpp"
#include "can_gen/CanRegistry.hpp"

bool isAllowedToSend(CanModule module, CanMessageId msg) {
    const MessageDefinition* def = getMessageDefinition(msg);
    if (def == nullptr) {
        return false;
    }
    return def->owner == module;
}