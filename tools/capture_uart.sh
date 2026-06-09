#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 4 ]; then
    echo "Usage:"
    echo "  s1: $0 <port> s1 <variant:512|768|1024> <pie:pieon|pieoff> <reconn|session> [baud]"
    echo "  s2: $0 <port> s2 <variant:512|768|1024> <pie:pieon|pieoff> [baud]"
    exit 1
fi

PORT="$1"
SCENARIO="$2"
VARIANT="$3"
PIE_ARG="${4:-}"
ARG5="${5:-}"
ARG6="${6:-}"

case "$PIE_ARG" in
    pieon|pieoff)
        PIE_TAG="$PIE_ARG"
        ;;
    pie|on)
        PIE_TAG="pieon"
        ;;
    nopie|off)
        PIE_TAG="pieoff"
        ;;
    *)
        if [[ "$PIE_ARG" =~ ^[0-9]+$ ]]; then
            PIE_TAG="pieoff"
            echo "PIE flag missing, defaulting to pieoff" >&2
        else
            echo "Invalid pie flag: $PIE_ARG (use pieon or pieoff)"
            exit 1
        fi
        ;;
esac

# Parse optional reconnect tag (Scenario 1 only) and baud.
# Arg5 can be: reconn|session (reconnect tag), a number (baud, legacy), or empty.
RECONN_TAG=""
BAUD_ARG=""
if [[ "$ARG5" == "reconn" || "$ARG5" == "session" ]]; then
    RECONN_TAG="$ARG5"
    BAUD_ARG="$ARG6"
elif [[ "$ARG5" =~ ^[0-9]+$ ]]; then
    BAUD_ARG="$ARG5"
elif [[ -n "$ARG5" ]]; then
    echo "Invalid arg5: $ARG5 (expected reconn, session, baud number, or empty)"
    exit 1
fi

# For Scenario 1, reconnect tag is required.
if [[ "$SCENARIO" == "s1" && -z "$RECONN_TAG" ]]; then
    echo "Error: Scenario 1 requires a reconnect tag as arg5 (reconn or session)"
    echo "  Example: $0 $PORT s1 $VARIANT $PIE_TAG reconn"
    exit 1
fi

BAUD="${BAUD_ARG:-921600}"

case "$SCENARIO" in
    s1|s2) ;;
    *)
        echo "Invalid scenario: $SCENARIO (use s1 or s2)"
        exit 1
        ;;
esac

case "$VARIANT" in
    512|768|1024) ;;
    *)
        echo "Invalid variant: $VARIANT (use 512, 768, or 1024)"
        exit 1
        ;;
esac

if [ -z "${IDF_PATH:-}" ] && [ -f "$HOME/esp/esp-idf/export.sh" ]; then
    # Best-effort ESP-IDF environment setup.
    # shellcheck source=/dev/null
    source "$HOME/esp/esp-idf/export.sh" >/dev/null 2>&1
fi

TS="$(date +"%Y%m%d_%H%M%S")"
RAW_DIR="data/raw"
CSV_DIR="data/csv"

# Build filename: include reconnect tag only for Scenario 1.
if [[ -n "$RECONN_TAG" ]]; then
    FILE_BASE="${SCENARIO}_${VARIANT}_${PIE_TAG}_${RECONN_TAG}_${TS}"
else
    FILE_BASE="${SCENARIO}_${VARIANT}_${PIE_TAG}_${TS}"
fi
RAW_LOG="$RAW_DIR/${FILE_BASE}.log"
CSV_OUT="$CSV_DIR/${FILE_BASE}.csv"

mkdir -p "$RAW_DIR" "$CSV_DIR"

echo "Capturing UART to: $RAW_LOG"
echo "Filtered CSV to:  $CSV_OUT"
echo "Stop capture with Ctrl+] in monitor or Ctrl+C here."

idf.py -p "$PORT" -b "$BAUD" monitor \
    | sed -u 's/\x1b\[[0-9;]*m//g' \
    | tee "$RAW_LOG" \
    | awk -v s="$SCENARIO" 'BEGIN{pattern="^" s ","} /^scenario,/ {print; fflush(); next} $0 ~ pattern {print; fflush()}' \
    > "$CSV_OUT"
