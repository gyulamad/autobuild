#!/usr/bin/env bash
# =============================================================================
# check_coverage.sh — Verify test coverage meets a minimum threshold
# =============================================================================
# Usage:
#   ./check_coverage.sh [COVERAGE_INFO_FILE] [MIN_THRESHOLD_PERCENT]
#
# Arguments:
#   COVERAGE_INFO_FILE  Path to lcov .info file (default: .build/coverage.info)
#   MIN_THRESHOLD       Minimum acceptable line coverage % (default: 90)
#
# Exit codes:
#   0 — Coverage meets or exceeds the threshold.
#   1 — Coverage is below the threshold, or an error occurred.

set -euo pipefail

# --- Defaults ---------------------------------------------------------------
COVERAGE_FILE="${1:-.build/coverage.info}"
MIN_THRESHOLD="${2:-90}"

# --- Colors (ANSI-C quoting produces real ESC bytes) -----------------------
if [[ -t 1 ]]; then
    GREEN=$'\e[32m'
    RED=$'\e[31m'
    BOLD=$'\e[1m'
    RESET=$'\e[0m'
else
    # No colour when piped / redirected (CI-friendly)
    GREEN="" ;  RED="" ;  BOLD="" ;  RESET=""
fi

# --- Validate inputs --------------------------------------------------------
if [[ ! -f "$COVERAGE_FILE" ]]; then
    echo "${RED}❌ Coverage file not found: ${COVERAGE_FILE}${RESET}"
    echo "   Run the builder with --mode=test,coverage --run first."
    exit 1
fi

# --- Extract line coverage percentage ---------------------------------------
SUMMARY=$(lcov --summary "$COVERAGE_FILE" 2>&1) || {
    echo "${RED}❌ lcov failed to read: ${COVERAGE_FILE}${RESET}"
    echo "$SUMMARY"
    exit 1
}

LINE_PCT_LINE=$(echo "$SUMMARY" | grep 'lines\.\.')

if [[ -z "$LINE_PCT_LINE" ]]; then
    echo "${RED}❌ Could not parse line coverage from lcov summary.${RESET}"
    echo "--- lcov output ---"
    echo "$SUMMARY"
    exit 1
fi

# Extract the numeric percentage (e.g. "80.0%" → 80.0)
LINE_PCT=$(echo "$LINE_PCT_LINE" | grep -oP '[\d.]+(?=%)')

if [[ -z "$LINE_PCT" ]]; then
    echo "${RED}❌ Failed to extract coverage percentage: ${LINE_PCT_LINE}${RESET}"
    exit 1
fi

# --- Compare against threshold -----------------------------------------------
RESULT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
    'BEGIN { print (pct >= min) ? "PASS" : "FAIL" }')

if [[ "$RESULT" == "PASS" ]]; then
    STATUS_COLOR=$GREEN
else
    STATUS_COLOR=$RED
fi

# --- Print report -----------------------------------------------------------
echo ""
echo "${BOLD}╔══════════════════════════════════════════════════╗${RESET}"
printf  "║ %-50s ║\n" "${BOLD}Coverage Threshold Check${RESET}"
echo "╠══════════════════════════════════════════════════╣"
printf  "║ %-50s ║\n" "File: $COVERAGE_FILE"
printf  "║ %-50s ║\n" "Threshold: ${MIN_THRESHOLD}%"

# Build the "Actual" line carefully using printf with %b to interpret escapes
ACTUAL_LINE=$(printf "%b Actual   : %b%s%%%b" \
    "$BOLD" "$STATUS_COLOR" "${LINE_PCT}" "$RESET")
printf  "║ %-50s ║\n" "$ACTUAL_LINE"

echo "╠══════════════════════════════════════════════════╣"

if [[ "$RESULT" == "PASS" ]]; then
    PASS_MSG=$(printf "%b✓ PASS — Coverage meets threshold (%s%%).%b" \
        "$GREEN" "${MIN_THRESHOLD}" "$RESET")
    printf  "║ %-50s ║\n" "$PASS_MSG"
else
    DEFICIT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
        'BEGIN { printf "%.1f", min - pct }')
    FAIL_MSG=$(printf "%b✗ FAIL — %s%% below threshold.%b" \
        "$RED" "${DEFICIT}" "$RESET")
    printf  "║ %-50s ║\n" "$FAIL_MSG"
fi

echo "╚══════════════════════════════════════════════════╝${RESET}"
echo ""

# --- Exit with appropriate code -----------------------------------------------
if [[ "$RESULT" == "PASS" ]]; then
    exit 0
else
    exit 1
fi

