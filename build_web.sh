#!/usr/bin/env bash
# Build Sky Fire Patrol as a WebAssembly app using Emscripten.
#
# Prerequisites:
#   - Emscripten SDK installed and activated  (https://emscripten.org/docs/getting_started/downloads.html)
#   - Source this script from the repo's Shooter/Shooter/ directory, or run:
#       cd /path/to/Shooter/Shooter && bash build_web.sh
#
# Output: build_web/index.html + index.js + index.wasm + index.data
# Serve the output folder with any static server, e.g.:
#   python3 -m http.server -d build_web 8080

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build_web"

echo "==> Configuring with emcmake …"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

emcmake cmake "$SCRIPT_DIR" \
    -DCMAKE_BUILD_TYPE=Release

echo "==> Building …"
emmake cmake --build . --parallel "$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

echo ""
echo "Done!  Output files are in: $BUILD_DIR"
echo ""
echo "To play locally, run one of:"
echo "  python3 -m http.server -d '$BUILD_DIR' 8080"
echo "  npx serve '$BUILD_DIR'"
echo "Then open http://localhost:8080"
