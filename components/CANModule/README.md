# CANModule

A modular, DBC-driven CAN library for FSAE embedded systems. Designed to be hardware-agnostic at the core, strongly typed through code generation, and safe for use in multi-task RTOS environments.

Currently targets **ESP32-S3 with ESP-IDF v6.0** via the TWAI peripheral, with a platform abstraction layer that allows porting to other hardware.

---

## Architecture

The library is split into three layers:

```
core/          Platform-independent interfaces and logic
generated/     Auto-generated from your DBC file (messages, codec, registry, state cache)
platform/      Hardware-specific implementations
  freertos/    FreeRTOS mutex (works on any MCU running FreeRTOS)
  espidf/      ESP32 TWAI driver and stack factory
```

The user-facing entry point is `CanStack`, which wires everything together. You never touch the driver, mutex, or codec directly.

---

## Prerequisites

- ESP-IDF v6.0+
- Python 3.10+ (for code generation)
- PyYAML (`pip install pyyaml`)
- A DBC file describing your vehicle CAN network
- An `ownership.yaml` config mapping DBC node names to module identifiers

---

## Project Setup

### 1. Add as a submodule

```bash
git submodule add https://github.com/Western-Formula-Racing/CANModule components/CANModule
git submodule update --init --recursive
```

### 2. Add to your CMakeLists.txt

```cmake
idf_component_register(
    SRCS
        core/src/CanInterface.cpp
        core/src/CanPermissions.cpp
        core/src/CanStack.cpp
        generated/src/CanCodec.cpp
        generated/src/CanStateCacheUpdate.cpp
        platform/freertos/src/FreeRtosMutex.cpp
        platform/espidf/src/TwaiDriver.cpp
        platform/espidf/src/TwaiStackFactory.cpp
    INCLUDE_DIRS
        core/include
        generated/include
        platform/freertos/include
        platform/espidf/include
    REQUIRES
        freertos
        esp_driver_twai
)
```

---

## Code Generation

The generated files (`CanMessages.hpp`, `CanCodec.hpp`, `CanStateCache.hpp`, etc.) are produced from your DBC file. **You must re-run the generator whenever the DBC changes.**

### File layout expected by the generator

```
your-project/
  dbc/
    WFR25.dbc          ← your DBC file
  config/
    ownership.yaml     ← module ownership config
  scripts/
    generate_can.py    ← the generator
```

### Running the generator

```bash
python scripts/generate_can.py
```

This writes all generated files into `generated/include/can_gen/` and `generated/src/`.

### ownership.yaml

This file maps DBC node names to `CanModule` enum values. It controls which module is allowed to transmit which messages.

```yaml
node_map:
  INV: MODULE_INV
  ORIONBMS: MODULE_ORIONBMS
  VCU: MODULE_ECU
  CUSTOMBMS: MODULE_BMS
  ELCON: MODULE_ELCON
  MOBO: MODULE_MOBO
  BASESTATION: MODULE_BASESTATION

# Override individual messages if the DBC sender is wrong
overrides:
  # PackStatus: MODULE_BMS
```

If you add a new node to the DBC, add it here and regenerate.

---

## Usage

### Single include

```cpp
#include "can/WFRCanModule.hpp"
```

This pulls in everything: core, generated, and platform headers.

---

### Setting up the CAN stack

`CanStack` is the top-level object. Create it once at startup using `CanStackConfig`.

```cpp
#include "can/WFRCanModule.hpp"

CanStack gCanStack;

void initCan() {
    CanStackConfig config;
    config.platform     = CanPlatform::EspTwai;
    config.module       = CanModule::MODULE_ECU;   // identity of THIS node
    config.txPin        = GPIO_NUM_5;
    config.rxPin        = GPIO_NUM_4;
    config.bitrate      = 500000;                  // 500 kbps
    config.rxQueueDepth = 32;

    gCanStack = CanStack::create(config);

    if (!gCanStack.isValid()) {
        ESP_LOGE("CAN", "Failed to create CAN stack");
        return;
    }

    if (!gCanStack.start()) {
        ESP_LOGE("CAN", "Failed to start CAN stack");
        return;
    }
}
```

**`CanStackConfig` fields:**

| Field | Default | Description |
|---|---|---|
| `platform` | `EspTwai` | Platform backend |
| `module` | `MODULE_UNKNOWN` | Identity of this node — must match ownership.yaml |
| `txPin` | `-1` | GPIO pin for CAN TX |
| `rxPin` | `-1` | GPIO pin for CAN RX |
| `bitrate` | `500000` | Bus bitrate in bits/second |
| `rxQueueDepth` | `32` | RX frame queue depth |
| `espidf.txQueueDepth` | `5` | TX hardware queue depth |
| `espidf.failRetryCount` | `1` | TX retry count on failure |
| `espidf.busOffIndicatorPin` | `-1` | Optional GPIO for bus-off indicator |
| `espidf.enableListenOnly` | `false` | Monitor-only mode, no TX |

---

### Receiving messages (RX task)

Create a dedicated FreeRTOS task that calls `receiveAnyAndUpdate()` in a loop. This is the only task that writes to the internal state cache.

```cpp
void rxTask(void* arg) {
    while (true) {
        // Blocks for up to 10ms waiting for a frame.
        // Decodes and writes to the internal cache under mutex protection.
        gCanStack.can().receiveAnyAndUpdate(10);
    }
}

// In your app_main or init:
xTaskCreate(rxTask, "can_rx", 4096, nullptr, 5, nullptr);
```

---

### Reading received data (any task)

Any task that wants to read CAN state calls `snapshot()`. This takes the mutex, copies the entire cache, and releases immediately. You then work on the local copy — no lock held.

```cpp
void appTask(void* arg) {
    while (true) {
        // Get a consistent point-in-time copy of all received messages
        CanStateCache state = gCanStack.can().snapshot();

        // Check validity before using a value
        if (state.m167VoltageInfoValid) {
            float voltage = state.m167VoltageInfo.iNVDCBusVoltage;
            ESP_LOGI("APP", "DC Bus Voltage: %.1f V", voltage);
        }

        if (state.m165MotorPositionInfoValid) {
            int16_t rpm = state.m165MotorPositionInfo.iNVMotorSpeed;
            ESP_LOGI("APP", "Motor Speed: %d RPM", rpm);
        }

        if (state.packStatusValid) {
            float packVoltage = state.packStatus.packSumVoltage;
            float soc         = state.packStatus.packSOC;
            ESP_LOGI("APP", "Pack: %.2f V, SOC: %.1f%%", packVoltage, soc);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

**Every message in `CanStateCache` has two fields:**
- The message struct (e.g. `m167VoltageInfo`) containing all decoded signal values
- A `bool` validity flag (e.g. `m167VoltageInfoValid`) that is `true` once at least one frame has been received

Always check the validity flag before using a value, especially at startup.

---

### Sending messages (TX)

Use the typed `send<T>()` method. The library enforces that only the module declared as owner in `ownership.yaml` can transmit a given message. Attempting to send a message you don't own returns `CanError::NotAllowed`.

```cpp
// Build the message struct
M192CommandMessage cmd{};
cmd.vCUINVInverterEnable  = true;
cmd.vCUINVDirectionCommand = false;
cmd.vCUINVTorqueCommand   = 50.0f;   // Nm

// Send it — encoding and frame construction happen internally
CanError err = gCanStack.can().send(cmd);

if (err != CanError::Ok) {
    ESP_LOGE("CAN", "Send failed: %d", static_cast<int>(err));
}
```

**`CanError` values:**

| Value | Meaning |
|---|---|
| `Ok` | Frame transmitted successfully |
| `NotAllowed` | This module does not own this message |
| `UnknownMessage` | Message ID not found in the registry |
| `InvalidFrame` | DLC or ID mismatch against the registry |
| `DriverError` | Driver rejected the frame (bus-off, queue full, etc.) |

---

### Typical RTOS task structure

```
┌─────────────────┐     receiveAnyAndUpdate()     ┌──────────────────┐
│    RX Task      │ ──────────────────────────►   │  CanStateCache   │
│  (priority 5)   │   writes under mutex           │  (internal)      │
└─────────────────┘                                └──────────────────┘
                                                           │
                                              snapshot()   │
                                                           ▼
┌─────────────────┐                           ┌──────────────────────┐
│  App/Control    │ ◄─────────────────────── │  Local copy of cache │
│  Task           │   lock-free read          │  (stack variable)    │
└─────────────────┘                           └──────────────────────┘

┌─────────────────┐     send<T>()
│    TX Task      │ ──────────────────────────► CAN bus
│                 │
└─────────────────┘
```

- **One RX task** calls `receiveAnyAndUpdate()` exclusively
- **All other tasks** read via `snapshot()` — never touch the cache directly
- **TX tasks** call `send<T>()` from any task context

---

## Generated types

### Message structs

Each DBC message becomes a struct in `CanMessages.hpp`. Signal types are determined by their DBC definition:

| Condition | C++ type |
|---|---|
| 1-bit unsigned | `bool` |
| Scale ≠ 1.0 or offset ≠ 0.0 | `float` (physical value, already scaled) |
| Unsigned, no scale/offset | `uint8_t` / `uint16_t` / `uint32_t` |
| Signed, no scale/offset | `int8_t` / `int16_t` / `int32_t` |

Example:

```cpp
struct M172TorqueAndTimerInfoMessage {
    float iNVPowerOnTimer    = {};   // scaled signal → float
    float iNVTorqueFeedback  = {};
    float iNVCommandedTorque = {};
};

struct M165MotorPositionInfoMessage {
    int16_t iNVMotorSpeed       = {};  // signed, no scale → int16_t
    float   iNVElectricalAngle  = {};
    int16_t iNVFastMotorSpeed   = {};
};
```

You never encode or decode manually. The library handles all bit packing, scaling, and offset application internally.

---

## Bus health diagnostics

The driver tracks bus health counters accessible via the `TwaiDriver`:

```cpp
// Access the driver through the stack (cast required)
auto& driver = static_cast<TwaiDriver&>(gCanStack.driver());

uint32_t dropped  = driver.getDroppedCount();  // RX frames lost (queue overflow)
uint32_t errors   = driver.getErrorCount();    // RX extraction errors
uint32_t busOffs  = driver.getBusOffCount();   // Bus-off events (auto-recovery attempted)
```

Bus-off recovery is automatic — when detected via the `on_state_change` callback, `twai_node_recover()` is called immediately and `busOffCount_` is incremented. Monitor `getBusOffCount()` from your fault logger.

---

## Porting to a new platform

The core and generated layers have no platform dependency. To port:

1. Implement `ICanDriver` for your hardware's CAN peripheral
2. Implement `ICanMutex` if not using FreeRTOS (a `FreeRtosMutex` is already provided in `platform/freertos/`)
3. Write a factory function analogous to `createEspTwaiStack()` that wires your driver and mutex into a `CanStack`
4. Register your sources and includes in the build system

The `CanStack`, `CanInterface`, `CanStateCache`, and all generated code remain unchanged.

---

## File reference

| File | Layer | Description |
|---|---|---|
| `core/include/can/CanInterface.hpp` | Core | High-level send/receive/snapshot API |
| `core/include/can/CanStack.hpp` | Core | Top-level user-facing stack object |
| `core/include/can/CanStackConfig.hpp` | Core | Hardware-agnostic configuration struct |
| `core/include/can/CanError.hpp` | Core | Send error codes |
| `core/include/can/ICanDriver.hpp` | Core | Driver interface |
| `core/include/can/ICanMutex.hpp` | Core | Mutex interface |
| `generated/include/can_gen/CanMessages.hpp` | Generated | Typed message structs |
| `generated/include/can_gen/CanStateCache.hpp` | Generated | State cache struct |
| `generated/include/can_gen/CanModule.hpp` | Generated | Module ownership enum |
| `generated/include/can_gen/CanCodec.hpp` | Generated | Encode/decode functions |
| `platform/freertos/include/FreeRtosMutex.hpp` | Platform | FreeRTOS mutex implementation |
| `platform/espidf/include/can_platform/TwaiDriver.hpp` | Platform | ESP32 TWAI driver |
| `platform/espidf/include/can_platform/TwaiStackFactory.hpp` | Platform | ESP32 stack factory |
| `core/include/WFRCanModule.hpp` | Convenience | Single include for everything |
| `scripts/generate_can.py` | Tooling | DBC → C++ code generator |
| `config/ownership.yaml` | Config | Message ownership mapping |