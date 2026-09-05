#!/usr/bin/env python3
"""Build and send firmware through this project's UART OTA protocol."""

from __future__ import annotations

import argparse
import shutil
import subprocess
import sys
from pathlib import Path


PROJECT_NAME = "glucose_monitor_espidf"


def project_root() -> Path:
    return Path(__file__).resolve().parents[1]


def run_command(command: list[str], cwd: Path) -> None:
    print("+ " + " ".join(command))
    subprocess.run(command, cwd=cwd, check=True)


def build_firmware(args: argparse.Namespace, root: Path) -> Path:
    idf_py = shutil.which("idf.py")
    build_dir_name = args.build_dir if args.build_dir is not None else ("build_ota_test" if args.test_app else "build")
    build_dir = Path(build_dir_name)

    if idf_py is None:
        raise RuntimeError("idf.py not found. Source ESP-IDF export.sh before running this script.")

    if not build_dir.is_absolute():
        build_dir = root / build_dir

    command = [idf_py, "-B", str(build_dir)]
    if args.test_app:
        command.extend(
            [
                "-DAPP_OTA_TEST_MODE=ON",
                f"-DAPP_OTA_TEST_VERSION={args.version}",
            ]
        )

    command.append("build")
    run_command(command, root)

    return build_dir / f"{PROJECT_NAME}.bin"


def send_firmware(args: argparse.Namespace, root: Path, firmware_path: Path) -> None:
    sender = root / "tools" / "ota_uart_send.py"
    command = [
        sys.executable,
        str(sender),
        "--port",
        args.port,
        "--file",
        str(firmware_path),
        "--baud",
        str(args.baud),
        "--chunk-size",
        str(args.chunk_size),
        "--timeout",
        str(args.timeout),
        "--retries",
        str(args.retries),
        "--settle-delay",
        str(args.settle_delay),
    ]
    run_command(command, root)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build and send firmware using UART OTA")
    parser.add_argument("-p", "--port", required=True, help="OTA UART serial port, e.g. /dev/ttyUSB1")
    parser.add_argument("-f", "--file", help="Existing firmware .bin to send")
    parser.add_argument("-b", "--baud", type=int, default=115200, help="UART baud rate")
    parser.add_argument("--build", action="store_true", help="Build firmware before sending")
    parser.add_argument("--test-app", action="store_true", help="Build the minimal OTA test firmware")
    parser.add_argument("--version", default="ota-test", help="Version label for --test-app build")
    parser.add_argument("--build-dir", help="ESP-IDF build directory")
    parser.add_argument("--chunk-size", type=int, default=256, help="Firmware payload bytes per DATA packet")
    parser.add_argument("--timeout", type=float, default=3.0, help="ACK/NACK timeout in seconds")
    parser.add_argument("--retries", type=int, default=3, help="Retry count per packet")
    parser.add_argument("--settle-delay", type=float, default=0.2, help="Delay after opening serial port")
    return parser.parse_args()


def main() -> int:
    status = 0
    args = parse_args()
    root = project_root()
    firmware_path = None

    try:
        if args.file is not None:
            firmware_path = Path(args.file)
            if not firmware_path.is_absolute():
                firmware_path = root / firmware_path
        elif args.build or args.test_app:
            firmware_path = build_firmware(args, root)
        else:
            firmware_path = root / "build" / f"{PROJECT_NAME}.bin"

        if not firmware_path.exists():
            raise FileNotFoundError(f"Firmware binary not found: {firmware_path}")

        send_firmware(args, root, firmware_path)
    except KeyboardInterrupt:
        print("\nAborted by user", file=sys.stderr)
        status = 130
    except Exception as exc:
        print(f"\nOTA demo failed: {exc}", file=sys.stderr)
        status = 1

    return status


if __name__ == "__main__":
    raise SystemExit(main())
