import re
from pathlib import Path
import yaml


BO_PATTERN = re.compile(r"^BO_\s+(\d+)\s+([A-Za-z0-9_]+):\s+(\d+)\s+([A-Za-z0-9_]+)")

SG_PATTERN = re.compile(
    r"^SG_\s+([A-Za-z0-9_]+)\s*:\s*"
    r"(\d+)\|(\d+)@([01])([+-])\s*"
    r"\(([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?),"
    r"([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\)"
)


def sanitize_name(name: str) -> str:
    name = re.sub(r'[^a-zA-Z0-9]', '_', name)
    name = re.sub(r'_+', '_', name)
    name = name.strip('_')

    if re.match(r'^\d', name):
        name = "_" + name

    return name.upper()


def get_can_id_info(raw_can_id: int) -> tuple[int, bool]:
    is_extended = raw_can_id > 0x7FF
    if is_extended:
        can_id = raw_can_id & 0x1FFFFFFF
    else:
        can_id = raw_can_id
    return can_id, is_extended


def parse_dbc_messages_and_signals(dbc_path: Path) -> list[dict]:
    messages = []
    current_message = None

    with dbc_path.open("r", encoding="utf-8") as f:
        for raw_line in f:
            line = raw_line.strip()

            bo_match = BO_PATTERN.match(line)
            if bo_match:
                current_message = {
                    "can_id": int(bo_match.group(1)),
                    "name": bo_match.group(2),
                    "dlc": int(bo_match.group(3)),
                    "sender": bo_match.group(4),
                    "signals": [],
                }
                messages.append(current_message)
                continue

            sg_match = SG_PATTERN.match(line)
            if sg_match and current_message is not None:
                signal = {
                    "name": sg_match.group(1),
                    "start_bit": int(sg_match.group(2)),
                    "bit_length": int(sg_match.group(3)),
                    "byte_order": int(sg_match.group(4)),   # 0 = Motorola, 1 = Intel
                    "is_signed": sg_match.group(5) == "-",
                    "scale": float(sg_match.group(6)),
                    "offset": float(sg_match.group(7)),
                }
                current_message["signals"].append(signal)

    return messages


def load_ownership_config(config_path: Path) -> dict:
    with config_path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f)


def should_skip_message(message: dict) -> bool:
    return (
        message["name"] == "VECTOR__INDEPENDENT_SIG_MSG"
        or message["sender"] == "Vector__XXX"
    )

# Messages whose cache entries carry a receive timestamp (LastReceivedMs).
TIMESTAMPED_PREFIXES = ("TORCH_M",)

def needs_timestamp(msg: dict) -> bool:
    return any(msg["name"].startswith(p) for p in TIMESTAMPED_PREFIXES)


def resolve_owner(message: dict, ownership_config: dict) -> str:
    overrides = ownership_config.get("overrides", {})
    node_map = ownership_config.get("node_map", {})

    if message["name"] in overrides:
        return overrides[message["name"]]

    sender = message["sender"]
    if sender not in node_map:
        return "MODULE_UNKNOWN"

    return node_map[sender]


def get_unique_modules(ownership_config: dict) -> list[str]:
    node_map = ownership_config.get("node_map", {})
    modules = sorted(set(node_map.values()))
    if "MODULE_UNKNOWN" not in modules:
        modules.append("MODULE_UNKNOWN")
    return modules


def to_struct_name(name: str) -> str:
    parts = re.split(r'[^a-zA-Z0-9]+', name)
    parts = [p for p in parts if p]

    base = "".join(part[:1].upper() + part[1:] for part in parts)

    if base.endswith("Message"):
        return base
    return base + "Message"


def to_field_name(name: str) -> str:
    parts = re.split(r'[^a-zA-Z0-9]+', name)
    parts = [p for p in parts if p]

    if not parts:
        return "unknownField"

    first = parts[0][:1].lower() + parts[0][1:]
    rest = [p[:1].upper() + p[1:] for p in parts[1:]]
    return first + "".join(rest)


def get_signal_cpp_type(signal: dict) -> str:
    if signal["bit_length"] == 1 and not signal["is_signed"]:
        return "bool"

    # If no scaling and no offset → raw integer
    if signal["scale"] == 1.0 and signal["offset"] == 0.0:
        if signal["bit_length"] <= 8:
            return "uint8_t" if not signal["is_signed"] else "int8_t"
        elif signal["bit_length"] <= 16:
            return "uint16_t" if not signal["is_signed"] else "int16_t"
        else:
            return "uint32_t" if not signal["is_signed"] else "int32_t"

    # Otherwise → physical value
    return "float"

def to_cache_field_name(message_name: str) -> str:
    parts = re.split(r'[^a-zA-Z0-9]+', message_name)
    parts = [p for p in parts if p]

    if not parts:
        return "unknownMessage"

    first = parts[0][:1].lower() + parts[0][1:]
    rest = [p[:1].upper() + p[1:] for p in parts[1:]]
    return first + "".join(rest)

##############################################################################
########### CODE GENERATION ##################################################
##############################################################################


def generate_can_module_hpp(modules: list[str], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Represents the logical source (owner) of a CAN message.")
    lines.append(" *")
    lines.append(" * Each module corresponds to a node or subsystem in the vehicle")
    lines.append(" * (e.g., ECU, BMS, inverter, dashboard). This is used to enforce")
    lines.append(" * transmission permissions so that only the correct module is")
    lines.append(" * allowed to send specific CAN messages.")
    lines.append(" */")
    lines.append("enum class CanModule : uint8_t {")
    for module in modules:
        lines.append(f"    {module},")
    lines.append("};")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")


def generate_message_id_hpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Unique identifiers for all CAN messages defined in the DBC.")
    lines.append(" *")
    lines.append(" * Each value corresponds to a specific CAN frame and is used")
    lines.append(" * throughout the system for encoding, decoding, and permission checks.")
    lines.append(" */")
    lines.append("enum class CanMessageId : uint16_t {")

    for msg in messages:
        clean_name = sanitize_name(msg["name"])
        lines.append(f"    CAN_MSG_{clean_name},")

    lines.append("};")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")


def generate_can_registry_hpp(messages: list[dict], ownership_config: dict, output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append("#include <cstddef>")
    lines.append("#include <cstdint>")
    lines.append('#include "CanModule.hpp"')
    lines.append('#include "CanMessageId.hpp"')
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Describes a CAN message defined in the system.")
    lines.append(" *")
    lines.append(" * This registry connects the generated message enum to its")
    lines.append(" * bus-level metadata, including CAN ID, DLC, frame type,")
    lines.append(" * and the module allowed to transmit it.")
    lines.append(" */")
    lines.append("struct MessageDefinition {")
    lines.append("    CanMessageId msg;")
    lines.append("    uint32_t canId;")
    lines.append("    uint8_t dlc;")
    lines.append("    bool extended;")
    lines.append("    CanModule owner;")
    lines.append("    const char* name;")
    lines.append("};")
    lines.append("")
    lines.append("inline constexpr MessageDefinition kMessageRegistry[] = {")

    for msg in messages:
        clean_name = sanitize_name(msg["name"])
        owner = resolve_owner(msg, ownership_config)
        can_id, is_extended = get_can_id_info(msg["can_id"])

        lines.append(
            f'    {{ CanMessageId::CAN_MSG_{clean_name}, {can_id}, {msg["dlc"]}, '
            f'{"true" if is_extended else "false"}, CanModule::{owner}, "{msg["name"]}" }},'
        )

    lines.append("};")
    lines.append("")
    lines.append("inline constexpr std::size_t kMessageRegistrySize = sizeof(kMessageRegistry) / sizeof(kMessageRegistry[0]);")
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Finds a message definition by its generated message ID.")
    lines.append(" *")
    lines.append(" * @param msg The generated CAN message ID.")
    lines.append(" * @return Pointer to the matching message definition, or nullptr if not found.")
    lines.append(" */")
    lines.append("inline constexpr const MessageDefinition* getMessageDefinition(CanMessageId msg) {")
    lines.append("    for (std::size_t i = 0; i < kMessageRegistrySize; i++) {")
    lines.append("        if (kMessageRegistry[i].msg == msg) {")
    lines.append("            return &kMessageRegistry[i];")
    lines.append("        }")
    lines.append("    }")
    lines.append("    return nullptr;")
    lines.append("}")
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Finds a message definition by its raw CAN ID.")
    lines.append(" *")
    lines.append(" * @param canId The bus-level CAN identifier.")
    lines.append(" * @return Pointer to the matching message definition, or nullptr if not found.")
    lines.append(" */")
    lines.append("inline constexpr const MessageDefinition* getMessageDefinitionByCanId(uint32_t canId) {")
    lines.append("    for (std::size_t i = 0; i < kMessageRegistrySize; i++) {")
    lines.append("        if (kMessageRegistry[i].canId == canId) {")
    lines.append("            return &kMessageRegistry[i];")
    lines.append("        }")
    lines.append("    }")
    lines.append("    return nullptr;")
    lines.append("}")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")


def generate_can_messages_hpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append("#include <cstdint>")
    lines.append("")
    lines.append('#include "CanMessageId.hpp"')
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Typed message structs generated from the DBC.")
    lines.append(" *")
    lines.append(" * Each struct represents the physical-value view of one CAN message.")
    lines.append(" * Boolean 1-bit signals are exposed as bool, and all other signals")
    lines.append(" * are exposed as float in this first-pass generator.")
    lines.append(" */")
    lines.append("")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        lines.append(f"struct {struct_name} {{")

        if not msg["signals"]:
            lines.append("    uint8_t reserved = 0;")
        else:
            for signal in msg["signals"]:
                cpp_type = get_signal_cpp_type(signal)
                field_name = to_field_name(signal["name"])
                lines.append(f"    {cpp_type} {field_name} = {{}};")

        lines.append("};")
        lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")

def generate_can_codec_hpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append('#include "can/CanFrame.hpp"')
    lines.append('#include "CanMessages.hpp"')
    lines.append("")

    lines.append("/**")
    lines.append(" * @brief Generated CAN encode/decode functions.")
    lines.append(" *")
    lines.append(" * Each message gets a typed encoder and decoder that convert")
    lines.append(" * between its generated struct and a raw CanFrame.")
    lines.append(" */")
    lines.append("")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        lines.append(f"bool encode{struct_name}(const {struct_name}& msg, CanFrame& frame);")
        lines.append(f"bool decode{struct_name}(const CanFrame& frame, {struct_name}& msg);")
        lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")

def generate_can_codec_cpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append('#include "can_gen/CanCodec.hpp"')
    lines.append('#include "can_gen/CanRegistry.hpp"')
    lines.append("")
    lines.append("#include <cmath>")
    lines.append("#include <cstdint>")
    lines.append("#include <cstring>")
    lines.append("")
    lines.append("namespace {")
    lines.append("")
    lines.append("constexpr uint64_t maskBits(uint32_t bitLength) {")
    lines.append("    return (bitLength >= 64U) ? ~0ULL : ((1ULL << bitLength) - 1ULL);")
    lines.append("}")
    lines.append("")
    lines.append("int64_t signExtend(uint64_t value, uint32_t bitLength) {")
    lines.append("    if (bitLength == 0U || bitLength >= 64U) {")
    lines.append("        return static_cast<int64_t>(value);")
    lines.append("    }")
    lines.append("    const uint64_t signBit = 1ULL << (bitLength - 1U);")
    lines.append("    if ((value & signBit) == 0ULL) {")
    lines.append("        return static_cast<int64_t>(value);")
    lines.append("    }")
    lines.append("    const uint64_t extended = value | (~maskBits(bitLength));")
    lines.append("    return static_cast<int64_t>(extended);")
    lines.append("}")
    lines.append("")
    lines.append("uint64_t extractIntelBits(const uint8_t data[8], uint32_t startBit, uint32_t bitLength) {")
    lines.append("    uint64_t raw = 0ULL;")
    lines.append("    uint32_t byteIndex = startBit / 8U;")
    lines.append("    uint32_t bitIndex = startBit % 8U;")
    lines.append("")
    lines.append("    for (uint32_t i = 0; i < bitLength; ++i) {")
    lines.append("        const uint64_t bit = (static_cast<uint64_t>((data[byteIndex] >> bitIndex) & 0x1U) << i);")
    lines.append("        raw |= bit;")
    lines.append("")
    lines.append("        ++bitIndex;")
    lines.append("        if (bitIndex == 8U) {")
    lines.append("            bitIndex = 0U;")
    lines.append("            ++byteIndex;")
    lines.append("        }")
    lines.append("    }")
    lines.append("")
    lines.append("    return raw;")
    lines.append("}")
    lines.append("")
    lines.append("void insertIntelBits(uint8_t data[8], uint32_t startBit, uint32_t bitLength, uint64_t rawValue) {")
    lines.append("    uint32_t byteIndex = startBit / 8U;")
    lines.append("    uint32_t bitIndex = startBit % 8U;")
    lines.append("    rawValue &= maskBits(bitLength);")
    lines.append("")
    lines.append("    for (uint32_t i = 0; i < bitLength; ++i) {")
    lines.append("        const uint8_t bit = static_cast<uint8_t>((rawValue >> i) & 0x1ULL);")
    lines.append("        data[byteIndex] &= static_cast<uint8_t>(~(1U << bitIndex));")
    lines.append("        data[byteIndex] |= static_cast<uint8_t>(bit << bitIndex);")
    lines.append("")
    lines.append("        ++bitIndex;")
    lines.append("        if (bitIndex == 8U) {")
    lines.append("            bitIndex = 0U;")
    lines.append("            ++byteIndex;")
    lines.append("        }")
    lines.append("    }")
    lines.append("}")
    lines.append("")
    lines.append("uint64_t extractMotorolaBits(const uint8_t data[8], uint32_t startBit, uint32_t bitLength) {")
    lines.append("    uint64_t raw = 0ULL;")
    lines.append("    uint32_t byteIndex = startBit / 8U;")
    lines.append("    uint32_t bitIndex = startBit % 8U;")
    lines.append("")
    lines.append("    for (uint32_t i = 0; i < bitLength; ++i) {")
    lines.append("        raw = (raw << 1U) | static_cast<uint64_t>((data[byteIndex] >> bitIndex) & 0x1U);")
    lines.append("")
    lines.append("        if (bitIndex == 0U) {")
    lines.append("            bitIndex = 7U;")
    lines.append("            ++byteIndex;")
    lines.append("        } else {")
    lines.append("            --bitIndex;")
    lines.append("        }")
    lines.append("    }")
    lines.append("")
    lines.append("    return raw;")
    lines.append("}")
    lines.append("")
    lines.append("void insertMotorolaBits(uint8_t data[8], uint32_t startBit, uint32_t bitLength, uint64_t rawValue) {")
    lines.append("    uint32_t byteIndex = startBit / 8U;")
    lines.append("    uint32_t bitIndex = startBit % 8U;")
    lines.append("    rawValue &= maskBits(bitLength);")
    lines.append("")
    lines.append("    for (uint32_t i = 0; i < bitLength; ++i) {")
    lines.append("        const uint32_t srcShift = bitLength - 1U - i;")
    lines.append("        const uint8_t bit = static_cast<uint8_t>((rawValue >> srcShift) & 0x1ULL);")
    lines.append("        data[byteIndex] &= static_cast<uint8_t>(~(1U << bitIndex));")
    lines.append("        data[byteIndex] |= static_cast<uint8_t>(bit << bitIndex);")
    lines.append("")
    lines.append("        if (bitIndex == 0U) {")
    lines.append("            bitIndex = 7U;")
    lines.append("            ++byteIndex;")
    lines.append("        } else {")
    lines.append("            --bitIndex;")
    lines.append("        }")
    lines.append("    }")
    lines.append("}")
    lines.append("")
    lines.append("} // namespace")
    lines.append("")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        enum_name = sanitize_name(msg["name"])

        # encode
        lines.append(f"bool encode{struct_name}(const {struct_name}& msg, CanFrame& frame) {{")
        lines.append(f"    const MessageDefinition* def = getMessageDefinition(CanMessageId::CAN_MSG_{enum_name});")
        lines.append("    if (def == nullptr) {")
        lines.append("        return false;")
        lines.append("    }")
        lines.append("")
        lines.append("    frame.id = def->canId;")
        lines.append("    frame.dlc = def->dlc;")
        lines.append("    frame.extended = def->extended;")
        lines.append("    std::memset(frame.data, 0, sizeof(frame.data));")
        lines.append("")

        for sig in msg["signals"]:
            field_name = to_field_name(sig["name"])
            cpp_type = get_signal_cpp_type(sig)
            start_bit = sig["start_bit"]
            bit_length = sig["bit_length"]
            byte_order = sig["byte_order"]
            scale = sig["scale"]
            offset = sig["offset"]
            is_signed = sig["is_signed"]

            lines.append("    {")
            if cpp_type == "bool":
                lines.append(f"        const uint64_t rawValue = msg.{field_name} ? 1ULL : 0ULL;")
            elif cpp_type == "float":
                lines.append(f"        const double physicalValue = static_cast<double>(msg.{field_name});")
                if scale == 0.0:
                    lines.append("        return false;")
                else:
                    lines.append(f"        const double rawDouble = (physicalValue - ({offset})) / ({scale});")
                    if is_signed:
                        lines.append("        const int64_t rawSigned = static_cast<int64_t>(std::llround(rawDouble));")
                        lines.append("        const uint64_t rawValue = static_cast<uint64_t>(rawSigned);")
                    else:
                        lines.append("        const uint64_t rawValue = static_cast<uint64_t>(std::llround(rawDouble));")
            elif cpp_type.startswith("int"):
                lines.append(f"        const int64_t rawSigned = static_cast<int64_t>(msg.{field_name});")
                lines.append("        const uint64_t rawValue = static_cast<uint64_t>(rawSigned);")
            else:
                lines.append(f"        const uint64_t rawValue = static_cast<uint64_t>(msg.{field_name});")

            if byte_order == 1:
                lines.append(f"        insertIntelBits(frame.data, {start_bit}U, {bit_length}U, rawValue);")
            else:
                lines.append(f"        insertMotorolaBits(frame.data, {start_bit}U, {bit_length}U, rawValue);")
            lines.append("    }")

        lines.append("")
        lines.append("    return true;")
        lines.append("}")
        lines.append("")

        # decode
        lines.append(f"bool decode{struct_name}(const CanFrame& frame, {struct_name}& msg) {{")
        lines.append(f"    const MessageDefinition* def = getMessageDefinition(CanMessageId::CAN_MSG_{enum_name});")
        lines.append("    if (def == nullptr) {")
        lines.append("        return false;")
        lines.append("    }")
        lines.append("    if (frame.id != def->canId || frame.dlc != def->dlc || frame.extended != def->extended) {")
        lines.append("        return false;")
        lines.append("    }")
        lines.append("")

        for sig in msg["signals"]:
            field_name = to_field_name(sig["name"])
            cpp_type = get_signal_cpp_type(sig)
            start_bit = sig["start_bit"]
            bit_length = sig["bit_length"]
            byte_order = sig["byte_order"]
            scale = sig["scale"]
            offset = sig["offset"]
            is_signed = sig["is_signed"]

            lines.append("    {")
            if byte_order == 1:
                lines.append(f"        const uint64_t rawValue = extractIntelBits(frame.data, {start_bit}U, {bit_length}U);")
            else:
                lines.append(f"        const uint64_t rawValue = extractMotorolaBits(frame.data, {start_bit}U, {bit_length}U);")

            if cpp_type == "bool":
                lines.append(f"        msg.{field_name} = (rawValue != 0ULL);")
            elif cpp_type == "float":
                if is_signed:
                    lines.append(f"        const int64_t signedValue = signExtend(rawValue, {bit_length}U);")
                    lines.append(f"        msg.{field_name} = static_cast<float>((static_cast<double>(signedValue) * ({scale})) + ({offset}));")
                else:
                    lines.append(f"        msg.{field_name} = static_cast<float>((static_cast<double>(rawValue) * ({scale})) + ({offset}));")
            elif cpp_type.startswith("int"):
                lines.append(f"        msg.{field_name} = static_cast<{cpp_type}>(signExtend(rawValue, {bit_length}U));")
            else:
                lines.append(f"        msg.{field_name} = static_cast<{cpp_type}>(rawValue);")
            lines.append("    }")

        lines.append("")
        lines.append("    return true;")
        lines.append("}")
        lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")

def generate_can_message_traits_hpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append('#include "can/CanFrame.hpp"')
    lines.append('#include "CanMessageId.hpp"')
    lines.append('#include "CanMessages.hpp"')
    lines.append('#include "CanCodec.hpp"')
    lines.append("")
    lines.append("template <typename T>")
    lines.append("struct CanMessageTraits;")
    lines.append("")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        enum_name = sanitize_name(msg["name"])

        lines.append("template <>")
        lines.append(f"struct CanMessageTraits<{struct_name}> {{")
        lines.append(f"    static constexpr CanMessageId id = CanMessageId::CAN_MSG_{enum_name};")
        lines.append("")
        lines.append(f"    static bool encode(const {struct_name}& msg, CanFrame& frame) {{")
        lines.append(f"        return encode{struct_name}(msg, frame);")
        lines.append("    }")
        lines.append("")
        lines.append(f"    static bool decode(const CanFrame& frame, {struct_name}& msg) {{")
        lines.append(f"        return decode{struct_name}(frame, msg);")
        lines.append("    }")
        lines.append("};")
        lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")

def generate_can_state_cache_hpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append('#include "CanMessages.hpp"')
    lines.append('#include <cstdint>')
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Stores the latest decoded CAN message values.")
    lines.append(" *")
    lines.append(" * Each entry contains the most recently decoded version of a CAN")
    lines.append(" * message along with a validity flag indicating whether that")
    lines.append(" * message has been received successfully.")
    lines.append(" *")
    lines.append(" * Messages with receive timestamps also carry a LastReceivedMs field,")
    lines.append(" * set to the value of the monotonic clock (ms) when the frame arrived.")
    lines.append(" */")
    lines.append("struct CanStateCache {")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        field_name = to_cache_field_name(msg["name"])

        lines.append(f"    {struct_name} {field_name};")
        lines.append(f"    bool {field_name}Valid = false;")
        if needs_timestamp(msg):
            lines.append(f"    uint32_t {field_name}LastReceivedMs = 0U;")
        lines.append("")

    lines.append("};")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")

def generate_can_state_cache_update_hpp(output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("#pragma once")
    lines.append("")
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_can.py")
    lines.append("")
    lines.append('#include <cstdint>')
    lines.append('#include "can/CanFrame.hpp"')
    lines.append('#include "can_gen/CanStateCache.hpp"')
    lines.append("")
    lines.append("/**")
    lines.append(" * @brief Updates the state cache from a received CAN frame.")
    lines.append(" *")
    lines.append(" * @param frame      Incoming CAN frame")
    lines.append(" * @param stateCache Cache to update")
    lines.append(" * @param nowMs      Current monotonic time in milliseconds")
    lines.append(" * @return true if frame was recognized and processed")
    lines.append(" */")
    lines.append("bool updateStateCacheFromFrame(const CanFrame& frame, CanStateCache& stateCache, uint32_t nowMs);")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")


def generate_can_state_cache_update_cpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append('#include "can_gen/CanStateCacheUpdate.hpp"')
    lines.append('#include "can_gen/CanRegistry.hpp"')
    lines.append('#include "can_gen/CanCodec.hpp"')
    lines.append("")
    lines.append("bool updateStateCacheFromFrame(const CanFrame& frame, CanStateCache& stateCache, uint32_t nowMs) {")
    lines.append("    const MessageDefinition* def = getMessageDefinitionByCanId(frame.id);")
    lines.append("    if (def == nullptr) {")
    lines.append("        return false;")
    lines.append("    }")
    lines.append("")
    lines.append("    switch (def->msg) {")

    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        enum_name = sanitize_name(msg["name"])
        field_name = to_cache_field_name(msg["name"])

        lines.append(f"        case CanMessageId::CAN_MSG_{enum_name}: {{")
        lines.append(f"            {struct_name} decoded{{}};")
        lines.append(f"            if (decode{struct_name}(frame, decoded)) {{")
        lines.append(f"                stateCache.{field_name} = decoded;")
        lines.append(f"                stateCache.{field_name}Valid = true;")
        lines.append("                return true;")
        lines.append("            }")
        lines.append("            return false;")
        lines.append("        }")

    lines.append("        default:")
    lines.append("            return false;")
    lines.append("    }")
    lines.append("}")
    lines.append("")

    output_path.write_text("\n".join(lines), encoding="utf-8")






def main() -> None:
    base_dir = Path(__file__).resolve().parent.parent

    dbc_path = base_dir / "dbc" / "WFR_DBC.dbc"
    config_path = base_dir / "config" / "ownership.yaml"

    module_output_path = base_dir / "generated" / "include" / "can_gen" / "CanModule.hpp"
    message_id_output_path = base_dir / "generated" / "include" / "can_gen" / "CanMessageId.hpp"
    registry_output_path = base_dir / "generated" / "include" / "can_gen" / "CanRegistry.hpp"
    messages_output_path = base_dir / "generated" / "include" / "can_gen" / "CanMessages.hpp"
    codec_hpp_output_path = base_dir / "generated" / "include" / "can_gen" / "CanCodec.hpp"
    codec_cpp_output_path = base_dir / "generated" / "src" / "CanCodec.cpp"
    traits_output_path = base_dir / "generated" / "include" / "can_gen" / "CanMessageTraits.hpp"
    state_cache_output_path = base_dir / "generated" / "include" / "can_gen" / "CanStateCache.hpp"
    state_cache_update_hpp = base_dir / "generated" / "include" / "can_gen" / "CanStateCacheUpdate.hpp"
    state_cache_update_cpp = base_dir / "generated" / "src" / "CanStateCacheUpdate.cpp"


    messages = parse_dbc_messages_and_signals(dbc_path)
    ownership_config = load_ownership_config(config_path)

    filtered_messages = [msg for msg in messages if not should_skip_message(msg)]

    print(f"Found {len(filtered_messages)} usable messages:\n")
    for msg in filtered_messages[:10]:
        owner = resolve_owner(msg, ownership_config)
        print(
            f"id={msg['can_id']}, "
            f"name={msg['name']}, "
            f"dlc={msg['dlc']}, "
            f"sender={msg['sender']}, "
            f"owner={owner}"
        )

    modules = get_unique_modules(ownership_config)
    generate_can_module_hpp(modules, module_output_path)
    print(f"\nGenerated: {module_output_path}")

    generate_message_id_hpp(filtered_messages, message_id_output_path)
    print(f"Generated: {message_id_output_path}")

    generate_can_registry_hpp(filtered_messages, ownership_config, registry_output_path)
    print(f"Generated: {registry_output_path}")

    generate_can_messages_hpp(filtered_messages, messages_output_path)
    print(f"Generated: {messages_output_path}")

    generate_can_codec_hpp(filtered_messages, codec_hpp_output_path)
    print(f"Generated: {codec_hpp_output_path}")

    generate_can_codec_cpp(filtered_messages, codec_cpp_output_path)
    print(f"Generated: {codec_cpp_output_path}")

    generate_can_message_traits_hpp(filtered_messages, traits_output_path)
    print(f"Generated: {traits_output_path}")

    generate_can_state_cache_hpp(filtered_messages, state_cache_output_path)
    print(f"Generated: {state_cache_output_path}")

    generate_can_state_cache_update_hpp(state_cache_update_hpp)
    print(f"Generated: {state_cache_update_hpp}")

    generate_can_state_cache_update_cpp(filtered_messages, state_cache_update_cpp)
    print(f"Generated: {state_cache_update_cpp}")

if __name__ == "__main__":
    main()