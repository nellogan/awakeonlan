#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Only gather .cpp files; headers are analyzed via the translation units that include them
mapfile -d '' FILES < <(
  find "$ROOT/src" "$ROOT/tests" \
      -type f \
      -name '*.cpp' \
      -print0
)

if ((${#FILES[@]} == 0)); then
    echo "No source files found to lint."
    exit 0
fi

FIX=false
if [[ "${1:-}" == "--fix" ]] || [[ "${1:-}" == "fix" ]]; then
    FIX=true
fi

TIDY_ARGS=(
    -warnings-as-errors=*
)

if [ "$FIX" = true ]; then
    TIDY_ARGS+=(--fix)
    echo "Running clang-tidy with automatic fixes..."
else
    echo "Running clang-tidy check..."
fi

clang-tidy "${TIDY_ARGS[@]}" "${FILES[@]}" -- \
  -I"$ROOT/include" \
  -std=c++17 \
  -Wall \
  -Wextra \
  -Wconversion \
  -Wsign-conversion \
  -pedantic

echo "Linting complete!"
