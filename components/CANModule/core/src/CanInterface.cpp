#include "can/CanInterface.hpp"
#include "can/CanPermissions.hpp"
#include "can_gen/CanRegistry.hpp"

CanInterface::CanInterface(ICanDriver& driver, ICanMutex& mutex, IClock& clock, CanModule module)
    : driver_(driver), mutex_(mutex), clock_(clock), thisModule_(module) {}

CanError CanInterface::sendRaw(CanMessageId msg, const CanFrame& frame) {
    const MessageDefinition* def = getMessageDefinition(msg);
    if (def == nullptr) {
        return CanError::UnknownMessage;
    }

    if (!isAllowedToSend(thisModule_, msg)) {
        return CanError::NotAllowed;
    }

    if (def->canId != frame.id || def->dlc != frame.dlc || def->extended != frame.extended) {
        return CanError::InvalidFrame;
    }

    if (!driver_.transmit(frame)) {
        return CanError::DriverError;
    }

    return CanError::Ok;
}

bool CanInterface::receiveRaw(CanFrame& frame, uint32_t timeoutMs) {
    return driver_.receive(frame, timeoutMs);
}

bool CanInterface::receiveAnyAndUpdate(uint32_t timeoutMs) {
    CanFrame frame{};
    if (!driver_.receive(frame, timeoutMs)) {
        return false;
    }

    const uint32_t now = clock_.nowMs();   // ← replaces the direct esp_timer call

    mutex_.lock();
    const bool updated = updateStateCacheFromFrame(frame, cache_, now);
    mutex_.unlock();

    return updated;
}

CanStateCache CanInterface::snapshot() const {
    mutex_.lock();
    CanStateCache copy = cache_;
    mutex_.unlock();
    return copy;
}