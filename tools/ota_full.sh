#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${1:-}"
BIN_FILE="${2:-$PROJECT_DIR/build/glucose_monitor_espidf.bin}"

if [[ -z "$PORT" ]]; then
    echo "Usage: $0 <ota_uart_port> [firmware_bin]"
    echo "Example: $0 /dev/ttyUSB1"
    echo "Example: $0 /dev/ttyUSB1 build/glucose_monitor_espidf.bin"
    exit 1
fi

python3 "$PROJECT_DIR/tools/ota_demo_flash.py" \
    --port "$PORT" \
    --file "$BIN_FILE"
