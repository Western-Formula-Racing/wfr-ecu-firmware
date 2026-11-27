#pragma once

#include <memory>

#include "can/CanInterface.hpp"
#include "can/ICanMutex.hpp"
#include "can/CanStackConfig.hpp"

class ICanDriver;
class IClock;

/**
 * @brief Owns the low-level driver and high-level CAN interface.
 *
 * This is the user-facing stack object returned by CanStack::create().
 */
class CanStack {
public:
    CanStack();
    ~CanStack();

    CanStack(const CanStack&) = delete;
    CanStack& operator=(const CanStack&) = delete;

    CanStack(CanStack&& other) noexcept;
    CanStack& operator=(CanStack&& other) noexcept;

    /**
     * @brief Creates a CAN stack for the selected platform.
     *
     * If creation fails, the returned stack will be invalid.
     */
    static CanStack create(const CanStackConfig& config);

    /**
     * @brief Initializes the underlying driver.
     */
    bool init();

    /**
     * @brief Enables the underlying native CAN node/controller.
     */
    bool enable();

    /**
     * @brief Convenience helper for init() + enable().
     */
    bool start();

    /**
     * @brief Disables the underlying native CAN node/controller.
     */
    bool disable();

    /**
     * @brief Returns true if the stack was created successfully.
     */
    bool isValid() const;

    /**
     * @brief Access the high-level typed CAN interface.
     *
     * Caller must ensure isValid() is true before using this.
     */
    CanInterface& can();
    const CanInterface& can() const;

    /**
     * @brief Access the config used to create this stack.
     */
    const CanStackConfig& config() const;

private:
    using NativeBoolFn = bool (*)(void*);
    using NativeCleanupFn = void (*)(void*);

    CanStack(CanStackConfig config,
             std::unique_ptr<ICanDriver> driver,
             std::unique_ptr<ICanMutex> mutex,
             std::unique_ptr<IClock> clock,
             std::unique_ptr<CanInterface> can,
             void* nativeHandle,
             NativeBoolFn enableFn,
             NativeBoolFn disableFn,
             NativeCleanupFn cleanupFn);

    void reset();

    CanStackConfig config_{};
    std::unique_ptr<ICanDriver>   driver_;
    std::unique_ptr<ICanMutex>    mutex_;
    // Declared before can_ so it outlives it: can_ (via CanInterface) holds a
    // reference to *clock_, and members destroy in reverse declaration order.
    std::unique_ptr<IClock>       clock_;
    std::unique_ptr<CanInterface> can_;

    void* nativeHandle_ = nullptr;
    NativeBoolFn enableFn_ = nullptr;
    NativeBoolFn disableFn_ = nullptr;
    NativeCleanupFn cleanupFn_ = nullptr;

    friend CanStack createEspTwaiStack(const CanStackConfig& config);
};