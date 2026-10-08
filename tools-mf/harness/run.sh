#!/usr/bin/env bash
# Start the WASM harness with uv (not system python).
set -euo pipefail
cd "$(dirname "$0")"
if [[ ! -f vendor/mgba.js || ! -f vendor/mgba.wasm ]]; then
  ./fetch-mgba.sh
fi
exec uv run --python 3.12 --no-project python3 ./server.py "$@"
