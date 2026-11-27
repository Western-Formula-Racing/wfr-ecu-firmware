#include "can/CanStack.hpp"

#include <utility>

#include "can/ICanDriver.hpp"
#include "can/ICanMutex.hpp"
#include "can/IClock.hpp"
#include "can_platform/TwaiStackFactory.hpp"

CanStack::CanStack() = default;

CanStack::CanStack(CanStackConfig config,
                   std::unique_ptr<ICanDriver> driver,
                   std::unique_ptr<ICanMutex> mutex,
                   std::unique_ptr<IClock> clock,
                   std::unique_ptr<CanInterface> can,
                   void* nativeHandle,
                   NativeBoolFn enableFn,
                   NativeBoolFn disableFn,
                   NativeCleanupFn cleanupFn)
    : config_(config),
      driver_(std::move(driver)),
      mutex_(std::move(mutex)),
      clock_(std::move(clock)),
      can_(std::move(can)),
      nativeHandle_(nativeHandle),
      enableFn_(enableFn),
      disableFn_(disableFn),
      cleanupFn_(cleanupFn) {}

CanStack::~CanStack() {
    reset();
}

CanStack::CanStack(CanStack&& other) noexcept
    : config_(other.config_),
      driver_(std::move(other.driver_)),
      mutex_(std::move(other.mutex_)),
      clock_(std::move(other.clock_)),
      can_(std::move(other.can_)),
      nativeHandle_(other.nativeHandle_),
      enableFn_(other.enableFn_),
      disableFn_(other.disableFn_),
      cleanupFn_(other.cleanupFn_) {
    other.nativeHandle_ = nullptr;
    other.enableFn_ = nullptr;
    other.disableFn_ = nullptr;
    other.cleanupFn_ = nullptr;
}

CanStack& CanStack::operator=(CanStack&& other) noexcept {
    if (this != &other) {
        reset();

        config_ = other.config_;
        driver_ = std::move(other.driver_);
        mutex_  = std::move(other.mutex_);
        clock_  = std::move(other.clock_);
        can_    = std::move(other.can_);
        nativeHandle_ = other.nativeHandle_;
        enableFn_ = other.enableFn_;
        disableFn_ = other.disableFn_;
        cleanupFn_ = other.cleanupFn_;

        other.nativeHandle_ = nullptr;
        other.enableFn_ = nullptr;
        other.disableFn_ = nullptr;
        other.cleanupFn_ = nullptr;
    }
    return *this;
}

CanStack CanStack::create(const CanStackConfig& config) {
    switch (config.platform) {
        case CanPlatform::EspTwai:
            return createEspTwaiStack(config);

        default:
            return CanStack{};
    }
}

bool CanStack::init() {
    if (!driver_) {
        return false;
    }
    return driver_->init();
}

bool CanStack::enable() {
    if (nativeHandle_ == nullptr || enableFn_ == nullptr) {
        return false;
    }
    return enableFn_(nativeHandle_);
}

bool CanStack::start() {
    return init() && enable();
}

bool CanStack::disable() {
    if (nativeHandle_ == nullptr || disableFn_ == nullptr) {
        return false;
    }
    return disableFn_(nativeHandle_);
}

bool CanStack::isValid() const {
    return driver_ != nullptr &&
           can_ != nullptr &&
           nativeHandle_ != nullptr;
}

CanInterface& CanStack::can() {
    return *can_;
}

const CanInterface& CanStack::can() const {
    return *can_;
}

const CanStackConfig& CanStack::config() const {
    return config_;
}

void CanStack::reset() {
    can_.reset();     // holds a reference to *clock_ — tear down first
    clock_.reset();
    mutex_.reset();
    driver_.reset();

    if (nativeHandle_ != nullptr && cleanupFn_ != nullptr) {
        cleanupFn_(nativeHandle_);
    }

    nativeHandle_ = nullptr;
    enableFn_ = nullptr;
    disableFn_ = nullptr;
    cleanupFn_ = nullptr;
}