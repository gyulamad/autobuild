#!/bin/bash

# Get the directory where the script is located
SCRIPT_DIR="$(dirname "$(realpath "$0")")"

# Get the current working directory
CURRENT_DIR="$(pwd)"

# Store the current working directory
ORIGINAL_DIR="$CURRENT_DIR"
# Change to the script's directory
cd "$SCRIPT_DIR" || exit 1


# =========================================================

g++ autobuild.cpp --std=c++20 -o autobuild

# =========================================================

# Go back to the original
cd "$ORIGINAL_DIR" || exit 1
# echo "Returned to original directory: $(pwd)"