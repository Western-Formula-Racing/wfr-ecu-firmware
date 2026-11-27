import re
from pathlib import Path
import yaml


BO_PATTERN = re.compile(r"^BO_\s+(\d+)\s+([A-Za-z0-9_]+):\s+(\d+)\s+([A-Za-z0-9_]+)")
SG_PATTERN = re.compile(
    r"^SG_\s+([A-Za-z0-9_]+)\s*:\s*"
    r"(\d+)\|(\d+)@([01])([+-])\s*"
    r"\(([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?),"
    r"([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\)"
    r"(?:\s*\[([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\|([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\])?"
)


def sanitize_name(name: str) -> str:
    name = re.sub(r"[^a-zA-Z0-9]", "_", name)
    name = re.sub(r"_+", "_", name).strip("_")
    if re.match(r"^\d", name):
        name = "_" + name
    return name.upper()


def to_struct_name(name: str) -> str:
    parts = [p for p in re.split(r"[^a-zA-Z0-9]+", name) if p]
    base = "".join(part[:1].upper() + part[1:] for part in parts)
    return base if base.endswith("Message") else base + "Message"


def to_field_name(name: str) -> str:
    parts = [p for p in re.split(r"[^a-zA-Z0-9]+", name) if p]
    if not parts:
        return "unknownField"
    first = parts[0][:1].lower() + parts[0][1:]
    rest = [p[:1].upper() + p[1:] for p in parts[1:]]
    return first + "".join(rest)


def get_can_id_info(raw_can_id: int) -> tuple[int, bool]:
    is_extended = raw_can_id > 0x7FF
    can_id = raw_can_id & 0x1FFFFFFF if is_extended else raw_can_id
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
                    "byte_order": int(sg_match.group(4)),   # 0 Motorola, 1 Intel
                    "is_signed": sg_match.group(5) == "-",
                    "scale": float(sg_match.group(6)),
                    "offset": float(sg_match.group(7)),
                }
                # Parse optional range [min|max]
                if sg_match.group(8) is not None and sg_match.group(9) is not None:
                    signal["min_physical"] = float(sg_match.group(8))
                    signal["max_physical"] = float(sg_match.group(9))
                    # Clamp DBC ranges to what's physically possible
                    clamp_dbc_range_to_physical_limits(signal)
                current_message["signals"].append(signal)

    return messages


def clamp_dbc_range_to_physical_limits(signal: dict) -> None:
    """Validate and clamp DBC-specified ranges to what's physically possible."""
    bit_length = signal["bit_length"]
    is_signed = signal["is_signed"]
    scale = signal["scale"]
    offset = signal["offset"]
    
    # Calculate physical limits based on bit width
    if is_signed:
        min_raw = -(1 << (bit_length - 1))
        max_raw = (1 << (bit_length - 1)) - 1
    else:
        min_raw = 0
        max_raw = (1 << bit_length) - 1
    
    calc_min_physical = min_raw * scale + offset
    calc_max_physical = max_raw * scale + offset
    
    # Get DBC range before clamping
    dbc_min = signal.get("min_physical", calc_min_physical)
    dbc_max = signal.get("max_physical", calc_max_physical)
    
    # Clamp DBC ranges to calculated limits
    clamped_min = max(dbc_min, calc_min_physical)
    clamped_max = min(dbc_max, calc_max_physical)
    
    # If ranges are inverted or DBC is way off, discard it entirely
    # This catches bogus DBC ranges like [0|65] for a signal with offset 1720
    # This is for the ORIONBMS which we dont use but the DBC is weirdly bad and causes test generation failures
    if clamped_min > clamped_max:
        # Remove the DBC range, use calculated limits instead
        if "min_physical" in signal:
            del signal["min_physical"]
        if "max_physical" in signal:
            del signal["max_physical"]
    else:
        # Use clamped values
        if "min_physical" in signal:
            signal["min_physical"] = clamped_min
        if "max_physical" in signal:
            signal["max_physical"] = clamped_max


def should_skip_message(message: dict) -> bool:
    return (
        message["name"] == "VECTOR__INDEPENDENT_SIG_MSG"
        or message["sender"] == "Vector__XXX"
    )


def get_signal_cpp_type(signal: dict) -> str:
    if signal["bit_length"] == 1 and not signal["is_signed"]:
        return "bool"

    if signal["scale"] == 1.0 and signal["offset"] == 0.0:
        if signal["bit_length"] <= 8:
            return "int8_t" if signal["is_signed"] else "uint8_t"
        if signal["bit_length"] <= 16:
            return "int16_t" if signal["is_signed"] else "uint16_t"
        return "int32_t" if signal["is_signed"] else "uint32_t"

    return "float"


def cpp_value_literal(signal: dict, tier: str) -> str:
    cpp_type = get_signal_cpp_type(signal)
    bit_length = signal["bit_length"]
    is_signed = signal["is_signed"]
    scale = signal["scale"]
    offset = signal["offset"]

    if cpp_type == "bool":
        return "false" if tier in ("roundtrip_default", "boundary_low") else "true"

    if cpp_type.startswith("uint"):
        max_raw = min((1 << bit_length) - 1, (1 << 31) - 1)
        if tier in ("roundtrip_default", "boundary_low"):
            return "0"
        if tier == "boundary_high":
            return str(max_raw)
        return "0"

    if cpp_type.startswith("int"):
        min_raw = -(1 << (bit_length - 1))
        max_raw = (1 << (bit_length - 1)) - 1
        if tier == "roundtrip_default":
            return "0"
        if tier == "boundary_low":
            return str(min_raw)
        if tier == "boundary_high":
            return str(max_raw)
        return "0"

    # float / scaled physical-value signals
    # Use DBC-specified ranges if available, otherwise calculate from bit width
    if "min_physical" in signal and "max_physical" in signal:
        min_physical = signal["min_physical"]
        max_physical = signal["max_physical"]
    else:
        if is_signed:
            min_raw = -(1 << (bit_length - 1))
            max_raw = (1 << (bit_length - 1)) - 1
        else:
            min_raw = 0
            max_raw = (1 << bit_length) - 1

        min_physical = min_raw * scale + offset
        max_physical = max_raw * scale + offset

    if tier == "roundtrip_default":
        if min_physical <= 0.0 <= max_physical:
            value = 0.0
        else:
            value = min_physical

    elif tier == "boundary_low":
        value = min_physical

    elif tier == "boundary_high":
        value = max_physical

    else:
        value = min_physical

    return f"{value:.6f}f"


def compare_expr(struct_var_a: str, struct_var_b: str, signal: dict) -> str:
    field = to_field_name(signal["name"])
    cpp_type = get_signal_cpp_type(signal)

    if cpp_type == "bool" or cpp_type.startswith("uint") or cpp_type.startswith("int"):
        return f"assert({struct_var_a}.{field} == {struct_var_b}.{field});"

    scale = signal["scale"]
    offset = signal["offset"]

    return (
        f"assert(nearlyEqual("
        f"quantizePhysical({struct_var_a}.{field}, {scale}f, {offset}f), "
        f"{struct_var_b}.{field}"
        f"));"
    )


def generate_test_cpp(messages: list[dict], output_path: Path) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)

    lines = []
    lines.append("// AUTO-GENERATED FILE. DO NOT EDIT.")
    lines.append("// Generated by scripts/generate_tests.py")
    lines.append("")
    lines.append("#include <cassert>")
    lines.append("#include <cmath>")
    lines.append("#include <cstdint>")
    lines.append("#include <iostream>")
    lines.append("")
    lines.append('#include "can/CanFrame.hpp"')
    lines.append('#include "can_gen/CanMessages.hpp"')
    lines.append('#include "can_gen/CanCodec.hpp"')
    lines.append('#include "can_gen/CanRegistry.hpp"')
    lines.append("")
    lines.append("static bool nearlyEqual(float a, float b, float eps = 0.001f) {")
    lines.append("    return std::fabs(a - b) < eps;")
    lines.append("}")
    lines.append("")
    lines.append("static float quantizePhysical(float physical, float scale, float offset) {")
    lines.append("    const double raw = std::llround((static_cast<double>(physical) - offset) / scale);")
    lines.append("    return static_cast<float>(raw * scale + offset);")
    lines.append("}")
    lines.append("")

    # Tier 1 helpers
    for msg in messages:
        if should_skip_message(msg):
            continue

        struct_name = to_struct_name(msg["name"])
        func_suffix = struct_name
        can_id, is_extended = get_can_id_info(msg["can_id"])

        # default fill
        lines.append(f"static {struct_name} makeDefault{func_suffix}() {{")
        lines.append(f"    {struct_name} msg{{}};")
        for sig in msg["signals"]:
            field = to_field_name(sig["name"])
            value = cpp_value_literal(sig, "roundtrip_default")
            lines.append(f"    msg.{field} = {value};")
        lines.append("    return msg;")
        lines.append("}")
        lines.append("")

        # low fill
        lines.append(f"static {struct_name} makeBoundaryLow{func_suffix}() {{")
        lines.append(f"    {struct_name} msg{{}};")
        for sig in msg["signals"]:
            field = to_field_name(sig["name"])
            value = cpp_value_literal(sig, "boundary_low")
            lines.append(f"    msg.{field} = {value};")
        lines.append("    return msg;")
        lines.append("}")
        lines.append("")

        # high fill
        lines.append(f"static {struct_name} makeBoundaryHigh{func_suffix}() {{")
        lines.append(f"    {struct_name} msg{{}};")
        for sig in msg["signals"]:
            field = to_field_name(sig["name"])
            value = cpp_value_literal(sig, "boundary_high")
            lines.append(f"    msg.{field} = {value};")
        lines.append("    return msg;")
        lines.append("}")
        lines.append("")

        # roundtrip default
        lines.append(f"static void testRoundTrip{func_suffix}() {{")
        lines.append(f"    {struct_name} tx = makeDefault{func_suffix}();")
        lines.append("    CanFrame frame{};")
        lines.append(f"    bool ok = encode{struct_name}(tx, frame);")
        lines.append("    assert(ok);")
        lines.append(f"    assert(frame.id == {can_id}U);")
        lines.append(f"    assert(frame.dlc == {msg['dlc']}U);")
        lines.append(f"    assert(frame.extended == {'true' if is_extended else 'false'});")
        lines.append(f"    {struct_name} rx{{}};")
        lines.append(f"    ok = decode{struct_name}(frame, rx);")
        lines.append("    assert(ok);")
        for sig in msg["signals"]:
            lines.append("    " + compare_expr("tx", "rx", sig))
        lines.append("}")
        lines.append("")

        # roundtrip low
        lines.append(f"static void testBoundaryLow{func_suffix}() {{")
        lines.append(f"    {struct_name} tx = makeBoundaryLow{func_suffix}();")
        lines.append("    CanFrame frame{};")
        lines.append(f"    bool ok = encode{struct_name}(tx, frame);")
        lines.append("    assert(ok);")
        lines.append(f"    {struct_name} rx{{}};")
        lines.append(f"    ok = decode{struct_name}(frame, rx);")
        lines.append("    assert(ok);")
        for sig in msg["signals"]:
            lines.append("    " + compare_expr("tx", "rx", sig))
        lines.append("}")
        lines.append("")

        # roundtrip high
        lines.append(f"static void testBoundaryHigh{func_suffix}() {{")
        lines.append(f"    {struct_name} tx = makeBoundaryHigh{func_suffix}();")
        lines.append("    CanFrame frame{};")
        lines.append(f"    bool ok = encode{struct_name}(tx, frame);")
        lines.append("    assert(ok);")
        lines.append(f"    {struct_name} rx{{}};")
        lines.append(f"    ok = decode{struct_name}(frame, rx);")
        lines.append("    assert(ok);")
        for sig in msg["signals"]:
            lines.append("    " + compare_expr("tx", "rx", sig))
        lines.append("}")
        lines.append("")

        # metadata failures
        lines.append(f"static void testMetadataFailures{func_suffix}() {{")
        lines.append(f"    {struct_name} tx = makeDefault{func_suffix}();")
        lines.append("    CanFrame frame{};")
        lines.append(f"    bool ok = encode{struct_name}(tx, frame);")
        lines.append("    assert(ok);")
        lines.append("")
        lines.append("    {")
        lines.append("        CanFrame bad = frame;")
        lines.append("        bad.id += 1U;")
        lines.append(f"        {struct_name} rx{{}};")
        lines.append(f"        ok = decode{struct_name}(bad, rx);")
        lines.append("        assert(!ok);")
        lines.append("    }")
        lines.append("")
        lines.append("    {")
        lines.append("        CanFrame bad = frame;")
        lines.append("        bad.dlc = static_cast<uint8_t>(bad.dlc + 1U);")
        lines.append(f"        {struct_name} rx{{}};")
        lines.append(f"        ok = decode{struct_name}(bad, rx);")
        lines.append("        assert(!ok);")
        lines.append("    }")
        lines.append("")
        lines.append("    {")
        lines.append("        CanFrame bad = frame;")
        lines.append("        bad.extended = !bad.extended;")
        lines.append(f"        {struct_name} rx{{}};")
        lines.append(f"        ok = decode{struct_name}(bad, rx);")
        lines.append("        assert(!ok);")
        lines.append("    }")
        lines.append("}")
        lines.append("")

    # category counters
    bool_msgs = []
    unsigned_msgs = []
    signed_msgs = []
    scaled_msgs = []
    motorola_msgs = []

    for msg in messages:
        if should_skip_message(msg):
            continue
        has_bool = any(get_signal_cpp_type(s) == "bool" for s in msg["signals"])
        has_unsigned = any(get_signal_cpp_type(s).startswith("uint") for s in msg["signals"])
        has_signed = any(get_signal_cpp_type(s).startswith("int") for s in msg["signals"])
        has_scaled = any(get_signal_cpp_type(s) == "float" for s in msg["signals"])
        has_motorola = any(s["byte_order"] == 0 for s in msg["signals"])
        name = to_struct_name(msg["name"])
        if has_bool:
            bool_msgs.append(name)
        if has_unsigned:
            unsigned_msgs.append(name)
        if has_signed:
            signed_msgs.append(name)
        if has_scaled:
            scaled_msgs.append(name)
        if has_motorola:
            motorola_msgs.append(name)

    lines.append("int main() {")
    lines.append('    std::cout << "Running generated CAN codec tests...\\n";')
    lines.append("")

    total_count = 0
    for msg in messages:
        if should_skip_message(msg):
            continue
        struct_name = to_struct_name(msg["name"])
        lines.append(f"    testRoundTrip{struct_name}();")
        lines.append(f"    testBoundaryLow{struct_name}();")
        lines.append(f"    testBoundaryHigh{struct_name}();")
        lines.append(f"    testMetadataFailures{struct_name}();")
        total_count += 4

    lines.append("")
    lines.append('    std::cout << "All generated tests passed.\\n";')
    lines.append(f'    std::cout << "Messages tested: {sum(1 for m in messages if not should_skip_message(m))}\\n";')
    lines.append(f'    std::cout << "Test blocks run: {total_count}\\n";')
    lines.append(f'    std::cout << "Messages with bool signals: {len(bool_msgs)}\\n";')
    lines.append(f'    std::cout << "Messages with unsigned integer signals: {len(unsigned_msgs)}\\n";')
    lines.append(f'    std::cout << "Messages with signed integer signals: {len(signed_msgs)}\\n";')
    lines.append(f'    std::cout << "Messages with scaled/float signals: {len(scaled_msgs)}\\n";')
    lines.append(f'    std::cout << "Messages with Motorola signals: {len(motorola_msgs)}\\n";')
    lines.append("    return 0;")
    lines.append("}")

    output_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    base_dir = Path(__file__).resolve().parent.parent
    dbc_path = base_dir / "dbc" / "WFR25.dbc"
    output_path = base_dir / "tests" / "generated" / "test_codec_generated.cpp"

    messages = parse_dbc_messages_and_signals(dbc_path)
    filtered_messages = [m for m in messages if not should_skip_message(m)]

    generate_test_cpp(filtered_messages, output_path)
    print(f"Generated: {output_path}")
    print(f"Usable messages: {len(filtered_messages)}")


if __name__ == "__main__":
    main()