#!/usr/bin/env bash
# Start the WASM harness with uv (not system python).
set -euo pipefail
cd "$(dirname "$0")"
if [[ ! -f vendor/mgba.js || ! -f vendor/mgba.wasm ]]; then
  ./fetch-mgba.sh
fi
# -u / PYTHONUNBUFFERED: "MF harness: http://…" must show up for AwaitShell
# (uv + Python 3 otherwise block-buffers stdout until the first request).
export PYTHONUNBUFFERED=1
exec uv run --python 3.12 --no-project python3 -u ./server.py "$@"
