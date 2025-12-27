#!/usr/bin/env bash

# Get parameters
BASE="$1"
TARGET="$2"
VERSION="$3"

# Validate input
if [ -z "$BASE" ] || [ -z "$TARGET" ] || [ -z "$VERSION" ]; then
    echo "Usage: $0 <base-folder> <target-folder> <version>"
    exit 1
fi

# Move into BASE directory
cd "$BASE" || exit 1

# Create directory if it does not exist
mkdir -p "$TARGET/$VERSION"

# Clone repository into that directory
git clone https://github.com/stevengj/nlopt.git "$TARGET/$VERSION"

# Checkout the requested version
cd "$TARGET/$VERSION" || exit 1
git checkout "$VERSION"

# Build
mkdir build
cd build || exit 1
cmake -DBUILD_SHARED_LIBS=OFF ..
make
