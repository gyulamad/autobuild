
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

rm -f *.o
rm -f *.test
rm -f *.gdb
rm -f *.cov
rm -f *.so
rm -f *.so.gdb
rm -f *.so.cov
rm -f *.so.gdb.cov
rm -f *.gcda
rm -f *.gcno

rm -f *.dep

# =========================================================

# Go back to the original
cd "$ORIGINAL_DIR" || exit 1
# echo "Returned to original directory: $(pwd)"