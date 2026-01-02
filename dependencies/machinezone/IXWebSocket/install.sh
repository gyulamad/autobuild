#!/usr/bin/env bash

# Get parameters
REPO="$1"
BASE="$2"
TARGET="$3"
VERSION="$4"

# Validate input
if [ -z "$BASE" ] || [ -z "$TARGET" ] || [ -z "$VERSION" ]; then
    echo "Usage: $0 <github-repo> <base-folder> <target-folder> <version>"
    exit 1
fi

# Move into BASE directory
echo "Going to base folder: $BASE"
cd "$BASE" || exit 1

# Create directory if it does not exist
echo "Creating lib folder: $TARGET/$VERSION"
mkdir -p "$TARGET/$VERSION"

# Clone repository into that directory
echo "Cloning repository: https://github.com/$REPO.git to $TARGET/$VERSION"
git clone "https://github.com/$REPO.git" "$TARGET/$VERSION"

# Checkout the requested version
echo "Going to folder: $TARGET/$VERSION and checkout $VERSION"
cd "$TARGET/$VERSION" || exit 1
git checkout "$VERSION"

# Build
echo "Building..."
mkdir build #make a build dir so that you can build out of tree
cd build
cmake -DUSE_TLS=1 ..
make -j
