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

# Save absolute path to target directory early (before any cd)
cd "$BASE" || exit 1
TARGET_DIR="$(pwd)/$TARGET/$VERSION"

mkdir -p "$TARGET/$VERSION"
echo "Cloning https://github.com/$REPO.git to $TARGET/$VERSION"
git clone "https://github.com/$REPO.git" "$TARGET/$VERSION" --recursive

# Checkout the requested branch (master-imgui_update for updated ImGui version)
cd "$TARGET_DIR" || exit 1
BRANCH="master-imgui_update"
if [ "$VERSION" != "" ] && [ "$VERSION" != "main" ]; then
    BRANCH="$VERSION"
fi

echo "Checking out branch: $BRANCH"
git checkout "$BRANCH" 2>/dev/null || echo "Warning: Could not checkout branch $BRANCH, using default"

# Re-initialize submodules after checkout (important for updated ImGui)
git submodule update --init --recursive

# Create stub headers for builder static analysis compatibility BEFORE building
# (ImGui conditionally includes these files when certain macros are defined,
# but the autobuild tool's static analyzer checks all #include directives regardless of preprocessor guards)
touch third-party/imgui/imgui/imgui_user.h
touch third-party/imgui/imgui/imgui_user.inl
touch third-party/imgui/imgui/stb_sprintf.h
touch third-party/imgui/imgui/stb_truetype.h
touch third-party/imgui/imgui/stb_rect_pack.h
touch third-party/imgui/imgui/stb_textedit.h

# Build imtui with ncurses support
echo "Building..."
mkdir -p build && cd build || exit 1
cmake -DBUILD_SHARED_LIBS=OFF \
    -DIMTUI_BUILD_EXAMPLES=OFF \
    -DIMTUI_SUPPORT_NCURSES=ON \
    -DIMTUI_SUPPORT_CURL=OFF ..
make -j$(nproc)
