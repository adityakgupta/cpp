#!/bin/bash

set -euo pipefail

case "${1:-0}" in
    "" | "r" | "0")
        BUILD_TYPE="Release"
        ;;
    "d" | "1")
        BUILD_TYPE="Debug"
        ;;
esac

DIR="$(dirname "${BASH_SOURCE[0]}")/build/$BUILD_TYPE"

rm -f "$DIR/../compile_commands.json"
rm -rf "$DIR"

cmake -S . -B "$DIR" -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE="$BUILD_TYPE" > /dev/null 2>&1
cp "$DIR/compile_commands.json" "$DIR/../compile_commands.json"

cmake --build "$DIR" --parallel > /dev/null 2>&1
