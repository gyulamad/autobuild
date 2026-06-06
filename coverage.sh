#!/usr/bin/env bash
# =============================================================================
# coverage.sh — Quick line coverage percentage check (filtered)
# Usage: ./coverage.sh <test_file.cpp> [min_percentage] [exclude_patterns]
#   e.g.: ./coverage.sh test.cpp 80                    (default exclusions applied)
#          ./coverage.sh my_tests.cpp                  (defaults for threshold + exclusions)
#          ./coverage.sh test.cpp 90 '.build/*'  (custom exclusions)
#
# Fails on any of these conditions:
#   - Test source file does not exist
#   - Tests do not pass during execution
#   - Line coverage is below the specified threshold
#
# Excludes external libraries and utility folders using builder's --coverage-exclude.
# This ensures exclusions are baked into both .info AND HTML reports at capture time.
# =============================================================================

set -euo pipefail

TEST_FILE="${1:-}"
MIN_THRESHOLD="${2:-50}"
EXCLUDE_PATTERNS="${3:-cpptools/misc/*,libs/*,autobuild/**}"
BUILD_DIR=".build/coverage-strict-test"
RAW_INFO="${BUILD_DIR}/coverage.info"

echo "============================================="
echo " Coverage Check — $(basename "$TEST_FILE")"
echo " Threshold : ${MIN_THRESHOLD}% line coverage"
echo " Excluding : $EXCLUDE_PATTERNS"
echo "============================================="
echo ""

# --- Verify dependencies ------------------------------------------------------
command -v lcov >/dev/null 2>&1 || { echo "ERROR: lcov is required. Install with: apt install lcov"; exit 1; }
[[ -x "./builder" ]] || { echo "ERROR: './builder' not found or not executable. Run from project root."; exit 1; }

# --- Validate test file exists -----------------------------------------------
if [[ -z "$TEST_FILE" ]]; then
    echo "Usage: $0 <test_file.cpp> [min_percentage] [exclude_patterns]"
    echo "  e.g.: $0 test.cpp 80"
    exit 1
fi

if [[ ! -f "$TEST_FILE" ]]; then
    echo "ERROR: Test file '$TEST_FILE' does not exist."
    exit 1
fi

# --- Build and run tests with coverage ----------------------------------------
echo "[1/3] Building & running tests with coverage..."

mkdir -p "${BUILD_DIR}"
BUILD_LOG="${BUILD_DIR}/build.log"

# Build and run - output shown in real-time via tee, saved to file for debugging
if ! ./builder "$TEST_FILE" --mode=coverage,strict,test --run \
    --coverage-exclude="$EXCLUDE_PATTERNS" 2>&1 | tee "$BUILD_LOG"; then
    
    echo ""
    echo "============================================="
    echo " BUILD/TEST FAILED (full log):"
    echo "============================================="
    cat "$BUILD_LOG"
    echo ""
    exit 1
fi

# Check if tests passed by looking for the success indicator in output
if ! grep -qE "(All.*test\(s\) passed|test\(s\) passed)" "$BUILD_LOG"; then
    
    echo "============================================="
    echo " BUILD/TEST OUTPUT (full log):"
    echo "============================================="
    cat "$BUILD_LOG"
    echo ""
    exit 1
fi

echo "[2/3] Tests passed."
echo ""

# --- Check coverage data exists -----------------------------------------------
if [[ ! -f "$RAW_INFO" ]]; then
    echo "ERROR: No coverage data found at $RAW_INFO"
    exit 1
fi

# --- Extract percentages from already-filtered .info --------------------------
LINE_PCT=$(lcov --summary "$RAW_INFO" 2>/dev/null | grep 'lines\.\.' | grep -oP '[\d.]+(?=%)' || echo "N/A")
FUNC_PCT=$(lcov --summary "$RAW_INFO" 2>/dev/null | grep 'functions\.\.' | grep -oP '[\d.]+(?=%)' || echo "N/A")

if [[ "$LINE_PCT" == "N/A" ]]; then
    echo "ERROR: Could not extract coverage percentage from filtered data."
    exit 1
fi

echo "--- Coverage Summary (filtered at capture time) ---"
echo "Line coverage : ${LINE_PCT}%"
echo "Function cov. : ${FUNC_PCT}%"
echo "Threshold     : ${MIN_THRESHOLD}% (lines)"
echo ""

# --- Compare against threshold ------------------------------------------------
RESULT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
    'BEGIN { print (pct >= min) ? "PASS" : "FAIL" }')

if [[ "$RESULT" == "PASS" ]]; then
    echo "*** Status: PASS — coverage meets threshold ***"
    
    # Show per-file breakdown from filtered data
    echo ""
    echo "--- Per-File Coverage ---"
    lcov --list "$RAW_INFO" 2>/dev/null || true
    
    # Green success banner at the end
    echo ""
    printf '\033[1;32m'
    echo "╔═════════════════════════════════════════════╗"
    echo "║   ✓ TESTS AND COVERAGE PASSED               ║"
    echo "╚═════════════════════════════════════════════╝"
    printf '\033[0m'
    
    exit 0
else
    DEFICIT=$(awk -v pct="$LINE_PCT" -v min="$MIN_THRESHOLD" \
        'BEGIN { printf "%.1f", min - pct }')
    echo "*** Status: FAIL — ${DEFICIT}% below threshold ***"
    
    # Red failure banner at the end
    echo ""
    printf '\033[1;31m'
    echo "╔═════════════════════════════════════════════╗"
    echo "║   ✗ TESTS AND COVERAGE FAILED               ║"
    echo "╚═════════════════════════════════════════════╝"
    printf '\033[0m'
    
    exit 1
fi
