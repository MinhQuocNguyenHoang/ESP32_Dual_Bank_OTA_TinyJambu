#!/usr/bin/env python3
"""Send an ESP32 firmware binary to this project's UART OTA protocol."""

from __future__ import annotations

import argparse
import struct
import sys
import time
import zlib
from pathlib import Path

try:
    import serial
except ImportError:  # pragma: no cover - user environment check
    serial = None


MAGIC = b"\xA5\x5A"
CMD_START_OTA = 0x01
CMD_DATA = 0x02
CMD_END_OTA = 0x03
CMD_ABORT = 0x04

RESP_ACK = 0x79
RESP_NACK = 0x1F

STATUS_OK = 0x00
STATUS_NAMES = {
    0x00: "OK",
    0x01: "CRC_ERROR",
    0x02: "SEQ_ERROR",
    0x03: "LENGTH_ERROR",
    0x04: "STATE_ERROR",
    0x05: "INTERNAL_ERROR",
}

MAX_PAYLOAD_SIZE = 256
HEADER_SIZE = 7
CRC_SIZE = 2
RESPONSE_PAYLOAD_SIZE = 1


class OtaProtocolError(RuntimeError):
    pass


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF

    for value in data:
        crc ^= value << 8
        for _ in range(8):
            if (crc & 0x8000) != 0:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF

    return crc


def build_frame(command: int, sequence: int, payload: bytes) -> bytes:
    if len(payload) > MAX_PAYLOAD_SIZE:
        raise ValueError(f"Payload too large: {len(payload)} > {MAX_PAYLOAD_SIZE}")

    header = MAGIC + struct.pack("<BHH", command, sequence, len(payload))
    crc = crc16_ccitt(header + payload)
    return header + payload + struct.pack("<H", crc)


def build_start_payload(image: bytes) -> bytes:
    image_size = len(image)
    image_crc32 = zlib.crc32(image) & 0xFFFFFFFF
    return struct.pack("<II", image_size, image_crc32)


def read_exact(port: serial.Serial, size: int, timeout_s: float) -> bytes:
    deadline = time.monotonic() + timeout_s
    data = bytearray()

    while len(data) < size and time.monotonic() < deadline:
        chunk = port.read(size - len(data))
        if chunk:
            data.extend(chunk)

    if len(data) != size:
        raise TimeoutError(f"Timed out while reading {size} bytes")

    return bytes(data)


def read_response(port: serial.Serial, expected_sequence: int, timeout_s: float) -> tuple[int, int]:
    deadline = time.monotonic() + timeout_s
    sync = bytearray()

    while time.monotonic() < deadline:
        byte = port.read(1)
        if not byte:
            continue

        sync.append(byte[0])
        if len(sync) > 2:
            sync.pop(0)

        if bytes(sync) != MAGIC:
            continue

        remaining_timeout = max(0.01, deadline - time.monotonic())
        rest = read_exact(port, HEADER_SIZE - len(MAGIC), remaining_timeout)
        command, sequence, payload_len = struct.unpack("<BHH", rest)

        if payload_len != RESPONSE_PAYLOAD_SIZE:
            sync.clear()
            continue

        remaining_timeout = max(0.01, deadline - time.monotonic())
        payload_crc = read_exact(port, payload_len + CRC_SIZE, remaining_timeout)
        payload = payload_crc[:payload_len]
        received_crc = struct.unpack("<H", payload_crc[payload_len:])[0]
        frame_without_crc = MAGIC + rest + payload
        calculated_crc = crc16_ccitt(frame_without_crc)

        if received_crc != calculated_crc:
            sync.clear()
            continue

        if command not in (RESP_ACK, RESP_NACK):
            sync.clear()
            continue

        if sequence != expected_sequence:
            raise OtaProtocolError(
                f"Unexpected response sequence {sequence}, expected {expected_sequence}"
            )

        return command, payload[0]

    raise TimeoutError("Timed out while waiting for ACK/NACK")


def send_frame_with_retry(
    port: serial.Serial,
    frame: bytes,
    sequence: int,
    retries: int,
    timeout_s: float,
) -> None:
    last_error = "no response"

    for attempt in range(1, retries + 2):
        port.reset_input_buffer()
        port.write(frame)
        port.flush()

        try:
            response, status = read_response(port, sequence, timeout_s)
            status_name = STATUS_NAMES.get(status, f"UNKNOWN_{status:02X}")
            if response == RESP_ACK and status == STATUS_OK:
                return

            last_error = f"NACK status={status_name}"
        except (TimeoutError, OtaProtocolError) as exc:
            last_error = str(exc)

        if attempt <= retries:
            print(f"\nRetry seq={sequence} attempt={attempt}/{retries}: {last_error}")

    raise OtaProtocolError(f"Frame seq={sequence} failed: {last_error}")


def print_progress(sent: int, total: int) -> None:
    width = 32
    ratio = 1.0 if total == 0 else sent / total
    filled = int(width * ratio)
    bar = "#" * filled + "-" * (width - filled)
    percent = ratio * 100.0
    print(f"\r[{bar}] {percent:6.2f}% {sent}/{total} bytes", end="", flush=True)


def send_ota(args: argparse.Namespace) -> None:
    if serial is None:
        raise RuntimeError("Missing dependency: install pyserial with `pip install pyserial`")

    firmware_path = Path(args.file)
    image = firmware_path.read_bytes()
    image_crc32 = zlib.crc32(image) & 0xFFFFFFFF

    if len(image) == 0:
        raise ValueError("Firmware file is empty")

    if args.chunk_size < 1 or args.chunk_size > MAX_PAYLOAD_SIZE:
        raise ValueError(f"chunk-size must be in range 1..{MAX_PAYLOAD_SIZE}")

    print(f"Firmware : {firmware_path}")
    print(f"Size     : {len(image)} bytes")
    print(f"CRC32    : 0x{image_crc32:08X}")
    print(f"Port     : {args.port} @ {args.baud}")

    with serial.Serial(args.port, args.baud, timeout=0.05, write_timeout=args.timeout) as port:
        time.sleep(args.settle_delay)
        port.reset_input_buffer()
        port.reset_output_buffer()

        sequence = 0
        start_frame = build_frame(CMD_START_OTA, sequence, build_start_payload(image))
        send_frame_with_retry(port, start_frame, sequence, args.retries, args.timeout)

        sequence += 1
        sent = 0
        print_progress(sent, len(image))

        for offset in range(0, len(image), args.chunk_size):
            chunk = image[offset : offset + args.chunk_size]
            frame = build_frame(CMD_DATA, sequence, chunk)
            send_frame_with_retry(port, frame, sequence, args.retries, args.timeout)

            sent += len(chunk)
            sequence = (sequence + 1) & 0xFFFF
            print_progress(sent, len(image))

        end_frame = build_frame(CMD_END_OTA, sequence, b"")
        send_frame_with_retry(port, end_frame, sequence, args.retries, args.timeout)

    print("\nOTA transfer complete. ESP32 should verify, set boot partition, and reboot.")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Send firmware .bin to ESP32 UART OTA")
    parser.add_argument("-p", "--port", required=True, help="Serial port, e.g. /dev/ttyUSB1")
    parser.add_argument("-f", "--file", required=True, help="Firmware .bin path")
    parser.add_argument("-b", "--baud", type=int, default=115200, help="UART baud rate")
    parser.add_argument(
        "-c",
        "--chunk-size",
        type=int,
        default=MAX_PAYLOAD_SIZE,
        help=f"Firmware payload bytes per DATA packet, max {MAX_PAYLOAD_SIZE}",
    )
    parser.add_argument("-t", "--timeout", type=float, default=3.0, help="ACK/NACK timeout in seconds")
    parser.add_argument("-r", "--retries", type=int, default=3, help="Retry count per packet")
    parser.add_argument(
        "--settle-delay",
        type=float,
        default=0.2,
        help="Delay after opening serial port before sending",
    )
    return parser.parse_args()


def main() -> int:
    status = 0

    try:
        send_ota(parse_args())
    except KeyboardInterrupt:
        print("\nAborted by user", file=sys.stderr)
        status = 130
    except Exception as exc:
        print(f"\nOTA failed: {exc}", file=sys.stderr)
        status = 1

    return status


if __name__ == "__main__":
    raise SystemExit(main())
