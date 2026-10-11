#!/usr/bin/env python3
"""Fail if a FireRed *release* ELF still has debug/quickstart entry points."""

from __future__ import annotations

import argparse
import hashlib
import os
import shutil
import subprocess
import sys

FORBIDDEN_SYMBOLS = (
    "MfDebug_Action_OpenQa",
    "MfDebug_Action_OpenQaMatrix",
    "CreateQuickstartHud",
    "Debug_ShowMainMenu",
)


def find_nm() -> str:
    roots = []
    env = os.environ.get("DEVKITARM")
    if env:
        roots.append(env)
    roots.append("/opt/devkitpro/devkitARM")
    for root in roots:
        candidate = os.path.join(root, "bin", "arm-none-eabi-nm")
        if os.path.isfile(candidate):
            return candidate
    found = shutil.which("arm-none-eabi-nm")
    if found:
        return found
    raise FileNotFoundError(
        "arm-none-eabi-nm not found (set DEVKITARM or put the toolchain on PATH)"
    )


def nm_names(elf: str) -> set[str]:
    out = subprocess.check_output([find_nm(), elf], text=True)
    names = set()
    for line in out.splitlines():
        parts = line.split()
        if len(parts) >= 3:
            names.add(parts[-1])
    return names


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--elf", default="pokefirered-release.elf")
    p.add_argument("--gba", default="pokefirered-release.gba")
    args = p.parse_args()

    names = nm_names(args.elf)
    found = [s for s in FORBIDDEN_SYMBOLS if s in names]
    if found:
        print("release ELF still exports debug/quickstart symbols:", ", ".join(found), file=sys.stderr)
        return 1

    with open(args.gba, "rb") as f:
        data = f.read()
    digest = hashlib.sha256(data).hexdigest()
    print(f"release ok  size={len(data)}  sha256={digest}")
    print(f"forbidden symbols absent: {', '.join(FORBIDDEN_SYMBOLS)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
