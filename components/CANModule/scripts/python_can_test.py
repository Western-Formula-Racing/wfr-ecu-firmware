#!/usr/bin/env python3
"""
Simple CAN stack simulator/test harness for use before ESP32 hardware is available.

Features:
- Parses a DBC file
- Encodes/decodes CAN messages using DBC signal definitions
- Simulates a CAN bus using an in-memory queue
- Optionally checks ownership via ownership.yaml
- Runs round-trip tests on selected messages

Usage examples:
    python python_can_test.py --dbc WFR25.dbc --ownership ownership.yaml --list
    python python_can_test.py --dbc WFR25.dbc --ownership ownership.yaml --test VCU_PRECHARGE
    python python_can_test.py --dbc WFR25.dbc --ownership ownership.yaml --demo
"""

from __future__ import annotations

import argparse
import math
import queue
import random
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple

try:
    import yaml
except ImportError:
    yaml = None


BO_PATTERN = re.compile(r"^BO_\s+(\d+)\s+([A-Za-z0-9_]+):\s+(\d+)\s+([A-Za-z0-9_]+)")
SG_PATTERN = re.compile(
    r"^SG_\s+([A-Za-z0-9_]+)\s*:\s*"
    r"(\d+)\|(\d+)@([01])([+-])\s*"
    r"\(([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?),"
    r"([-+]?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\)"
)


@dataclass
class SignalDefinition:
    name: str
    start_bit: int
    bit_length: int
    byte_order: int   # 0 = Motorola, 1 = Intel
    is_signed: bool
    scale: float
    offset: float


@dataclass
class MessageDefinition:
    raw_can_id: int
    can_id: int
    extended: bool
    name: str
    dlc: int
    sender: str
    signals: List[SignalDefinition] = field(default_factory=list)


@dataclass
class CanFrame:
    can_id: int
    dlc: int
    data: bytes
    extended: bool = False

    def __post_init__(self) -> None:
        if self.dlc > 8:
            raise ValueError("Classic CAN DLC must be <= 8")
        if len(self.data) != self.dlc:
            raise ValueError(f"Data length {len(self.data)} does not match DLC {self.dlc}")

    def pretty(self) -> str:
        data_hex = " ".join(f"{b:02X}" for b in self.data)
        return f"id=0x{self.can_id:X} dlc={self.dlc} ext={self.extended} data=[{data_hex}]"


class VirtualCanBus:
    def __init__(self) -> None:
        self._q: "queue.Queue[CanFrame]" = queue.Queue()

    def send(self, frame: CanFrame) -> None:
        self._q.put(frame)

    def receive(self, timeout: Optional[float] = None) -> CanFrame:
        return self._q.get(timeout=timeout)


def get_can_id_info(raw_can_id: int) -> Tuple[int, bool]:
    is_extended = raw_can_id > 0x7FF
    if is_extended:
        return raw_can_id & 0x1FFFFFFF, True
    return raw_can_id, False


def parse_dbc_messages(dbc_path: Path) -> Dict[str, MessageDefinition]:
    messages: Dict[str, MessageDefinition] = {}
    current: Optional[MessageDefinition] = None

    with dbc_path.open("r", encoding="utf-8") as f:
        for raw_line in f:
            line = raw_line.strip()

            bo_match = BO_PATTERN.match(line)
            if bo_match:
                raw_can_id = int(bo_match.group(1))
                can_id, extended = get_can_id_info(raw_can_id)
                current = MessageDefinition(
                    raw_can_id=raw_can_id,
                    can_id=can_id,
                    extended=extended,
                    name=bo_match.group(2),
                    dlc=int(bo_match.group(3)),
                    sender=bo_match.group(4),
                )
                messages[current.name] = current
                continue

            sg_match = SG_PATTERN.match(line)
            if sg_match and current is not None:
                sig = SignalDefinition(
                    name=sg_match.group(1),
                    start_bit=int(sg_match.group(2)),
                    bit_length=int(sg_match.group(3)),
                    byte_order=int(sg_match.group(4)),
                    is_signed=(sg_match.group(5) == "-"),
                    scale=float(sg_match.group(6)),
                    offset=float(sg_match.group(7)),
                )
                current.signals.append(sig)

    return messages


def load_ownership_config(path: Optional[Path]) -> Dict[str, Any]:
    if path is None:
        return {}
    if yaml is None:
        raise RuntimeError("PyYAML is required to load ownership.yaml. Install with: pip install pyyaml")
    with path.open("r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def resolve_owner(msg: MessageDefinition, ownership_config: Dict[str, Any]) -> str:
    if not ownership_config:
        return "UNKNOWN"

    overrides = ownership_config.get("overrides", {}) or {}
    node_map = ownership_config.get("node_map", {}) or {}

    if msg.name in overrides:
        return overrides[msg.name]

    return node_map.get(msg.sender, "MODULE_UNKNOWN")


def mask_bits(bit_length: int) -> int:
    if bit_length >= 64:
        return (1 << 64) - 1
    return (1 << bit_length) - 1


def sign_extend(value: int, bit_length: int) -> int:
    if bit_length == 0 or bit_length >= 64:
        return value
    sign_bit = 1 << (bit_length - 1)
    if value & sign_bit:
        return value | (~mask_bits(bit_length))
    return value


def extract_intel_bits(data: bytes, start_bit: int, bit_length: int) -> int:
    raw = 0
    byte_index = start_bit // 8
    bit_index = start_bit % 8

    for i in range(bit_length):
        bit = ((data[byte_index] >> bit_index) & 0x1) << i
        raw |= bit
        bit_index += 1
        if bit_index == 8:
            bit_index = 0
            byte_index += 1

    return raw


def insert_intel_bits(buffer: bytearray, start_bit: int, bit_length: int, raw_value: int) -> None:
    raw_value &= mask_bits(bit_length)
    byte_index = start_bit // 8
    bit_index = start_bit % 8

    for i in range(bit_length):
        bit = (raw_value >> i) & 0x1
        buffer[byte_index] &= ~(1 << bit_index)
        buffer[byte_index] |= bit << bit_index
        bit_index += 1
        if bit_index == 8:
            bit_index = 0
            byte_index += 1


def extract_motorola_bits(data: bytes, start_bit: int, bit_length: int) -> int:
    raw = 0
    byte_index = start_bit // 8
    bit_index = start_bit % 8

    for _ in range(bit_length):
        raw = (raw << 1) | ((data[byte_index] >> bit_index) & 0x1)
        if bit_index == 0:
            bit_index = 7
            byte_index += 1
        else:
            bit_index -= 1

    return raw


def insert_motorola_bits(buffer: bytearray, start_bit: int, bit_length: int, raw_value: int) -> None:
    raw_value &= mask_bits(bit_length)
    byte_index = start_bit // 8
    bit_index = start_bit % 8

    for i in range(bit_length):
        src_shift = bit_length - 1 - i
        bit = (raw_value >> src_shift) & 0x1
        buffer[byte_index] &= ~(1 << bit_index)
        buffer[byte_index] |= bit << bit_index
        if bit_index == 0:
            bit_index = 7
            byte_index += 1
        else:
            bit_index -= 1


def physical_to_raw(sig: SignalDefinition, physical_value: Any) -> int:
    if sig.bit_length == 1 and not sig.is_signed:
        return 1 if bool(physical_value) else 0

    if sig.scale == 0:
        raise ValueError(f"Signal {sig.name} has zero scale")

    raw_double = (float(physical_value) - sig.offset) / sig.scale
    raw_signed = int(round(raw_double))

    if sig.is_signed:
        return raw_signed & mask_bits(sig.bit_length)
    return raw_signed


def raw_to_physical(sig: SignalDefinition, raw_value: int) -> Any:
    if sig.bit_length == 1 and not sig.is_signed:
        return raw_value != 0

    if sig.is_signed:
        signed_value = sign_extend(raw_value, sig.bit_length)
        return (float(signed_value) * sig.scale) + sig.offset
    return (float(raw_value) * sig.scale) + sig.offset


def encode_message(msg_def: MessageDefinition, values: Dict[str, Any]) -> CanFrame:
    data = bytearray(msg_def.dlc)

    for sig in msg_def.signals:
        if sig.name not in values:
            raise KeyError(f"Missing value for signal: {sig.name}")

        raw_value = physical_to_raw(sig, values[sig.name])

        if sig.byte_order == 1:
            insert_intel_bits(data, sig.start_bit, sig.bit_length, raw_value)
        else:
            insert_motorola_bits(data, sig.start_bit, sig.bit_length, raw_value)

    return CanFrame(
        can_id=msg_def.can_id,
        dlc=msg_def.dlc,
        data=bytes(data),
        extended=msg_def.extended,
    )


def decode_message(msg_def: MessageDefinition, frame: CanFrame) -> Dict[str, Any]:
    if frame.can_id != msg_def.can_id:
        raise ValueError(f"CAN ID mismatch: got 0x{frame.can_id:X}, expected 0x{msg_def.can_id:X}")
    if frame.dlc != msg_def.dlc:
        raise ValueError(f"DLC mismatch: got {frame.dlc}, expected {msg_def.dlc}")
    if frame.extended != msg_def.extended:
        raise ValueError(f"Frame format mismatch: got ext={frame.extended}, expected ext={msg_def.extended}")

    decoded: Dict[str, Any] = {}
    for sig in msg_def.signals:
        if sig.byte_order == 1:
            raw_value = extract_intel_bits(frame.data, sig.start_bit, sig.bit_length)
        else:
            raw_value = extract_motorola_bits(frame.data, sig.start_bit, sig.bit_length)

        decoded[sig.name] = raw_to_physical(sig, raw_value)

    return decoded


def floats_close(a: Any, b: Any, tol: float = 1e-4) -> bool:
    if isinstance(a, bool) or isinstance(b, bool):
        return a == b
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return math.isclose(float(a), float(b), rel_tol=tol, abs_tol=tol)
    return a == b


def round_trip_test(msg_def: MessageDefinition, values: Dict[str, Any]) -> bool:
    frame = encode_message(msg_def, values)
    decoded = decode_message(msg_def, frame)

    ok = True
    for sig_name, expected in values.items():
        actual = decoded.get(sig_name)
        if not floats_close(expected, actual):
            print(f"  FAIL {sig_name}: expected={expected!r}, actual={actual!r}")
            ok = False

    if ok:
        print(f"  PASS round-trip: {msg_def.name}")
        print(f"  {frame.pretty()}")
    return ok


def simulate_bus_test(bus: VirtualCanBus, msg_def: MessageDefinition, values: Dict[str, Any]) -> bool:
    frame = encode_message(msg_def, values)
    bus.send(frame)
    rx = bus.receive(timeout=1.0)
    decoded = decode_message(msg_def, rx)

    ok = True
    for sig_name, expected in values.items():
        if not floats_close(expected, decoded[sig_name]):
            print(f"  FAIL bus {sig_name}: expected={expected!r}, actual={decoded[sig_name]!r}")
            ok = False

    if ok:
        print(f"  PASS bus simulation: {msg_def.name}")
        print(f"  {rx.pretty()}")
    return ok


def sample_values_for_message(msg_def: MessageDefinition) -> Dict[str, Any]:
    values: Dict[str, Any] = {}

    for sig in msg_def.signals:
        if sig.bit_length == 1 and not sig.is_signed:
            values[sig.name] = True
            continue

        if sig.scale == 1.0 and sig.offset == 0.0:
            if sig.is_signed:
                max_mag = min((1 << (sig.bit_length - 1)) - 1, 10)
                values[sig.name] = max_mag
            else:
                values[sig.name] = min((1 << sig.bit_length) - 1, 10)
            continue

        # Try to pick a simple physical value that survives round-trip scaling cleanly
        if sig.is_signed:
            raw = 5
        else:
            raw = 10
        values[sig.name] = (raw * sig.scale) + sig.offset

    return values


def print_message_summary(msg: MessageDefinition, owner: str) -> None:
    print(f"{msg.name}:")
    print(f"  CAN ID   : 0x{msg.can_id:X}")
    print(f"  DLC      : {msg.dlc}")
    print(f"  Extended : {msg.extended}")
    print(f"  Sender   : {msg.sender}")
    print(f"  Owner    : {owner}")
    print(f"  Signals  : {len(msg.signals)}")
    for sig in msg.signals:
        bo = "Intel" if sig.byte_order == 1 else "Motorola"
        sign = "signed" if sig.is_signed else "unsigned"
        print(
            f"    - {sig.name}: start={sig.start_bit} len={sig.bit_length} "
            f"{bo} {sign} scale={sig.scale} offset={sig.offset}"
        )


def run_demo(messages, ownership):
    preferred = [
        "VCU_Precharge",
        "VCU_State_Info",
        "VCU_Pedal_Info",
        "M192_Command_Message",
        "PackStatus",
    ]

    bus = VirtualCanBus()
    failures = 0

    for name in preferred:
        msg = messages.get(name)
        if msg is None:
            print(f"Skipping {name} (not found)")
            continue

        owner = resolve_owner(msg, ownership)
        print(f"\n=== {msg.name} (owner: {owner}) ===")

        values = sample_values_for_message(msg)

        if not round_trip_test(msg, values):
            failures += 1

        if not simulate_bus_test(bus, msg, values):
            failures += 1

    return 0 if failures == 0 else 2

def main() -> int:
    parser = argparse.ArgumentParser(description="Python CAN stack simulator/test harness")
    parser.add_argument("--dbc", required=True, type=Path, help="Path to DBC file")
    parser.add_argument("--ownership", type=Path, default=None, help="Optional ownership.yaml")
    parser.add_argument("--list", action="store_true", help="List all parsed messages")
    parser.add_argument("--show", type=str, help="Show one message definition")
    parser.add_argument("--test", type=str, help="Run round-trip and bus test on one message")
    parser.add_argument("--demo", action="store_true", help="Run demo tests on a few common messages")

    args = parser.parse_args()

    messages = parse_dbc_messages(args.dbc)
    ownership = load_ownership_config(args.ownership)

    if args.list:
        for name in sorted(messages):
            msg = messages[name]
            owner = resolve_owner(msg, ownership)
            print(f"{name:35s} id=0x{msg.can_id:<8X} dlc={msg.dlc} owner={owner}")
        return 0

    if args.show:
        msg = messages.get(args.show)
        if msg is None:
            print(f"Message not found: {args.show}", file=sys.stderr)
            return 1
        owner = resolve_owner(msg, ownership)
        print_message_summary(msg, owner)
        return 0

    if args.test:
        msg = messages.get(args.test)
        if msg is None:
            print(f"Message not found: {args.test}", file=sys.stderr)
            return 1

        owner = resolve_owner(msg, ownership)
        print(f"Testing {msg.name} (owner: {owner})")
        values = sample_values_for_message(msg)
        bus = VirtualCanBus()

        ok1 = round_trip_test(msg, values)
        ok2 = simulate_bus_test(bus, msg, values)
        return 0 if (ok1 and ok2) else 2

    if args.demo:
        return run_demo(messages, ownership)

    parser.print_help()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())