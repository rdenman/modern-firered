#!/usr/bin/env python3
"""Serve the WASM mGBA harness with COOP/COEP and the FireRed ROM."""

from __future__ import annotations

import argparse
import mimetypes
import sys
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

HARNESS_DIR = Path(__file__).resolve().parent
REPO_ROOT = HARNESS_DIR.parents[1]
ROM_PATH = REPO_ROOT / "pokefirered.gba"
VENDOR_DIR = HARNESS_DIR / "vendor"

mimetypes.add_type("application/wasm", ".wasm")
mimetypes.add_type("application/javascript", ".js")


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(HARNESS_DIR), **kwargs)

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_HEAD(self):
        if self.path.split("?", 1)[0] == "/rom":
            self._send_rom(head_only=True)
            return
        super().do_HEAD()

    def do_GET(self):
        if self.path.split("?", 1)[0] == "/rom":
            self._send_rom(head_only=False)
            return
        super().do_GET()

    def _send_rom(self, head_only: bool):
        if not ROM_PATH.is_file():
            self.send_error(
                404,
                f"Missing {ROM_PATH.name}. Build with: make firered -j$(sysctl -n hw.ncpu)",
            )
            return
        size = ROM_PATH.stat().st_size
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Length", str(size))
        self.end_headers()
        if not head_only:
            self.wfile.write(ROM_PATH.read_bytes())

    def log_message(self, fmt, *args):
        sys.stderr.write("%s - %s\n" % (self.address_string(), fmt % args))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", type=int, default=8765)
    args = parser.parse_args()

    missing = []
    if not (VENDOR_DIR / "mgba.js").is_file() or not (VENDOR_DIR / "mgba.wasm").is_file():
        missing.append(f"vendor — run: {HARNESS_DIR / 'fetch-mgba.sh'}")
    if not ROM_PATH.is_file():
        missing.append(f"ROM at {ROM_PATH} — run: make firered -j$(sysctl -n hw.ncpu)")
    if missing:
        print("Cannot start:\n  - " + "\n  - ".join(missing), file=sys.stderr)
        sys.exit(1)

    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"MF harness: http://127.0.0.1:{args.port}/", flush=True)
    print(f"ROM: {ROM_PATH} ({ROM_PATH.stat().st_size} bytes)", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nstopped")


if __name__ == "__main__":
    main()
