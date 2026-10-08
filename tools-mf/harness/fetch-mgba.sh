#!/usr/bin/env bash
# Pin @thenick775/mgba-wasm dist into vendor/ (gitignored; not a CDN).
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p vendor
dest="$PWD/vendor"
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT
cd "$tmpdir"
npm pack @thenick775/mgba-wasm@2.5.1 >/dev/null
tar -xzf thenick775-mgba-wasm-*.tgz
cp package/dist/mgba.js package/dist/mgba.wasm "$dest/"
echo "Wrote $dest/mgba.js and mgba.wasm"
