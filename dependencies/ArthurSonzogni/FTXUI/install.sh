#!/usr/bin/env bash

# Get parameters: REPO, BASE, TARGET, VERSION
REPO="$1"
BASE="$2"
TARGET="$3"
VERSION="$4"

if [ -z "$BASE" ] || [ -z "$TARGET" ] || [ -z "$VERSION" ]; then
    echo "Usage: $0 <github-repo> <base-folder> <target-folder> <version>"
    exit 1
fi

cd "$BASE" || exit 1
mkdir -p "$TARGET/$VERSION"
echo "Cloning https://github.com/$REPO.git to $TARGET/$VERSION"
git clone "https://github.com/$REPO.git" "$TARGET/$VERSION"

# Checkout the requested version (if not main/default)
cd "$TARGET/$VERSION" || exit 1
if [ "$VERSION" != "main" ]; then
    git checkout "$VERSION"
fi

# Build FTXUI (no install step — libs and headers end up in build/)
echo "Building..."
mkdir -p build && cd build || exit 1
cmake -DBUILD_SHARED_LIBS=OFF \
    -DFTXUI_BUILD_EXAMPLES=OFF \
    -DFTXUI_BUILD_DOCS=OFF \
    -DFTXUI_BUILD_TESTS=OFF ..
make -j$(nproc)

