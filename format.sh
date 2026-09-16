#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mapfile -d '' FILES < <(
  find "$ROOT/src" "$ROOT/include" "$ROOT/tests" \
      -type f \
      \( -name "*.cpp" -o -name "*.hpp" \) \
      -print0
)

if ((${#FILES[@]} == 0)); then
    echo "No source or header files found."
    exit 0
fi

if [[ "${1:-}" == "--check" ]] || [[ "${1:-}" == "check" ]]; then
    echo "Running clang-format dry-run check..."
    clang-format --dry-run --Werror "${FILES[@]}"
    echo "Format check passed!"
else
    echo "Running clang-format in-place..."
    clang-format -i "${FILES[@]}"
    echo "Formatting complete!"
fi
