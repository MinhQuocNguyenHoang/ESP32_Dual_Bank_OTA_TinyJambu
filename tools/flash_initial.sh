#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${1:-/dev/ttyUSB0}"

if ! command -v idf.py >/dev/null 2>&1; then
    if [[ -n "${IDF_PATH:-}" && -f "$IDF_PATH/export.sh" ]]; then
        source "$IDF_PATH/export.sh" >/dev/null
    elif [[ -f "$HOME/Tools/esp/esp-idf/export.sh" ]]; then
        source "$HOME/Tools/esp/esp-idf/export.sh" >/dev/null
    else
        echo "idf.py not found. Please source ESP-IDF export.sh first."
        exit 1
    fi
fi

idf.py -C "$PROJECT_DIR" -p "$PORT" flash monitor
