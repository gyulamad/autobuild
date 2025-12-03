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
echo "Compiling autobuild tool..."

#mkdir -p ../cpptools/misc
#git clone https://github.com/gyulamad/cpptools-misc.git ../cpptools/misc

TOOLS_DIR="../cpptools/misc"

if [ ! -d "$TOOLS_DIR" ]; then
  mkdir "$TOOLS_DIR"
  echo "Directory '$TOOLS_DIR' created."
  git clone https://github.com/gyulamad/cpptools-misc.git $TOOLS_DIR
else
  echo "Directory '$TOOLS_DIR' found."
fi
echo "Using '$TOOLS_DIR'"

g++ autobuild.cpp -rdynamic --std=c++20 -o "$ORIGINAL_DIR/builder"

# =========================================================

# Go back to the original
cd "$ORIGINAL_DIR" || exit 1
# echo "Returned to original directory: $(pwd)"