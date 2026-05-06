#!/usr/bin/env bash
# =============================================================================
# check_coverage.sh — Verify test coverage meets a minimum threshold
# =============================================================================
set -euo pipefail

COVERAGE_FILE="${1:-.build/coverage.info}"
MIN_THRESHOLD="${2:-90}"

TABLE_WIDTH=58  # inner content width between borders

# --- Colors ------------------------------------------------------------------
if [[ -t 1 ]]; then
    GREEN=$'\e[32m'; RED=$'\e[31m' ; YELLOW=$'\e[33m'
    BOLD=$'\e[1m'   ; DIM=$'\e[2m'  ; RESET=$'\e[0m'
else
    GREEN="" ; RED="" ; YELLOW="" ; BOLD="" ; DIM="" ; RESET=""
fi

# --- Helper: ANSI-safe table row ---------------------------------------------
# Strips escape sequences only to measure visible width, then pads with spaces.
_row() {
    local text="$1"
    # Strip standard SGR codes (\e[...m) for length measurement
    local vlen
    vlen=$(printf '%s' "$text" | sed $'s/\x1b\[[0-9;]*m//g' | wc -c)

    local pad=$(( TABLE_WIDTH - vlen ))
    (( pad < 0 )) && pad=0          # safety: don't go negative

    printf '║ %b%*s ║\n' "$text" "$pad" ""
}
_rowc() {
    local text="$1"
    # Strip standard SGR codes (\e[...m) for length measurement
    local vlen
    vlen=$(printf '%s' "$text" | sed $'s/\x1b\[[0-9;]*m//g' | wc -c)

    local pad=$(( TABLE_WIDTH - vlen ))
    (( pad < 0 )) && pad=0          # safety: don't go negative

    printf '║ %b%*s ║\n' "$text    " "$pad" ""
}

# --- Validate inputs ---------------------------------------------------------
if [[ ! -f "$COVERAGE_FILE" ]]; then
    echo "${RED}❌ Coverage file not found:${RESET} ${DIM}${COVERAGE_FILE}${RESET}"
    exit 1
fi

# --- Extract line coverage percentage ----------------------------------------
SUMMARY=$(lcov --summary "$COVERAGE_FILE" 2>&1) || {
    echo "${RED}❌ lcov failed to read: $COVERAGE_FILE${RESET}"
    echo "$SUMMARY" | sed 's/^/   /'
    exit 1
}

LINE_PCT_LINE=$(echo "$SUMMARY"     | grep 'lines\.\.' || true)

if [[ -z "$LINE_PCT_LINE" ]]; then
    echo "${RED}❌ Could not parse line coverage from lcov summary.${RESET}"
    exit 1
fi

# Extract numeric percentage (e.g. "80.2%" → 80.2)
LINE_PCT=$(echo "$LINE_PCT_LINE" | grep -oP '[\d.]+(?=%)' || true)

if [[ -z "$LINE_PCT" ]]; then
    echo "${RED}❌ Failed to extract coverage percentage.${RESET}"
    exit 1
fi

# --- Compare against threshold -----------------------------------------------
RESULT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
    'BEGIN { print (pct >= min) ? "PASS" : "FAIL" }')

if [[ "$RESULT" == "PASS" ]]; then
    STATUS_COLOR=$GREEN; ICON="✓"; MSG_TYPE="pass"
else
    STATUS_COLOR=$RED;   ICON="✗"; MSG_TYPE="fail"
fi

# --- Print report ------------------------------------------------------------
echo ""
printf "${BOLD}╔%s╗${RESET}\n" "$(printf '═%.0s' $(seq 1 $(( TABLE_WIDTH + 2 ))))"
_row "${BOLD}${DIM}Coverage Threshold Check${RESET}"
printf "╠%s╣\n" "$(printf '═%.0s' $(seq 1 $(( TABLE_WIDTH + 2 ))))"
_row "File      : ${DIM}$COVERAGE_FILE${RESET}"
_row "Threshold : ${BOLD}${MIN_THRESHOLD}%${RESET} line coverage"

# Actual coverage — color depends on pass/fail
_act="${STATUS_COLOR}${LINE_PCT}%${RESET}"
if [[ "$MSG_TYPE" == "pass" ]]; then
    _rowc "${DIM}Actual    : ${_act:0:1} ${BOLD}— Coverage meets threshold.${RESET}"
else
    DEFICIT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
        'BEGIN { printf "%.1f", min - pct }')
    _rowc "${DIM}Actual    : ${_act:0:1} ${RED}${BOLD}— ${DEFICIT}% below threshold.${RESET}"
fi

printf "╚%s╝\n\n" "$(printf '═%.0s' $(seq 1 $(( TABLE_WIDTH + 2 ))))"

# --- Exit --------------------------------------------------------------------
[[ "$RESULT" == "PASS" ]] && exit 0 || exit 1


