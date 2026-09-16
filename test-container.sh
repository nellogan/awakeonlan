#!/usr/bin/env bash
set -euo pipefail

echo "========================================"
echo "Starting pipeline for: x86_64"
echo "========================================"

echo "-> Building Docker x86_64 image..."
docker build -t awakeonlan-x86_64 . || \
{ echo "-> ERROR: Build failed for x86_64." >&2; exit 1; }
echo "-> Build successful."

echo "-> Running tests..."
docker run --rm awakeonlan-x86_64 /bin/bash -c \
    "make format-check && make lint && make CONFIG=xsan test && make CONFIG=msan test && make test-valgrind" || \
    { echo "-> ERROR: Tests failed for x86_64." >&2; exit 1; }
echo "-> SUCCESS: All tests passed for x86_64."

echo "========================================"
echo "Starting pipeline for: Aarch64"
echo "========================================"

echo "-> Building Docker arm64 image..."
docker buildx build --platform linux/arm64 -t awakeonlan-arm64 --load . || \
{ echo "-> ERROR: Build failed for arm64." >&2; exit 1; }
echo "-> Build successful."

# NOTE: MemorySanitizer (msan) is intentionally excluded here because it forces
# personality() / ADDR_NO_RANDOMIZE virtual memory layout requirements that
# fail under QEMU user-mode emulation.
# Additionally, LSAN_OPTIONS=detect_leaks=0 is passed because LeakSanitizer's
# thread-stopping (ptrace) is unsupported under emulation.
echo "-> Running tests..."
docker run --rm --platform linux/arm64 --init awakeonlan-arm64 /bin/bash -c \
    "make format-check && make lint && LSAN_OPTIONS=detect_leaks=0 make CONFIG=xsan test && make test-valgrind" || \
    { echo "-> ERROR: Tests failed for arm64." >&2; exit 1; }
echo "-> SUCCESS: All tests passed for Aarch64."

echo "========================================"
echo "All architectures completed successfully!"
echo "========================================"
