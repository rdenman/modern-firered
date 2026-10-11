#!/usr/bin/env python3
"""Create or apply BPS patches. Never ships a ROM — only a patch against one you provide.

Vanilla FireRed (USA, Rev 0 / BPRE-0), for `create`:
  size  16777216
  sha1  41cb43a279208d5e14ee8ae47285c6bdae16dd85
"""

from __future__ import annotations

import argparse
import hashlib
import io
import os
import sys
import tempfile
import zlib

BPS_MAGIC = b"BPS1"
VANILLA_FIRERED_USA_REV0_SIZE = 16_777_216
VANILLA_FIRERED_USA_REV0_SHA1 = "41cb43a279208d5e14ee8ae47285c6bdae16dd85"

SOURCE_READ = 0
TARGET_READ = 1


def crc32(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


def encode_number(n: int) -> bytes:
    out = bytearray()
    while True:
        x = n & 0x7F
        n >>= 7
        if n == 0:
            out.append(x | 0x80)
            break
        out.append(x)
        n -= 1
    return bytes(out)


def decode_number(buf: io.BytesIO) -> int:
    data = 0
    shift = 1
    while True:
        x = buf.read(1)
        if not x:
            raise ValueError("truncated BPS number")
        b = x[0]
        data += (b & 0x7F) * shift
        if b & 0x80:
            return data
        shift <<= 7
        data += shift


def encode_action(command: int, length: int) -> bytes:
    if length < 1:
        raise ValueError("BPS action length must be >= 1")
    return encode_number(((length - 1) << 2) + command)


def create_bps(source: bytes, target: bytes, metadata: bytes = b"") -> bytes:
    body = bytearray()
    body += BPS_MAGIC
    body += encode_number(len(source))
    body += encode_number(len(target))
    body += encode_number(len(metadata))
    body += metadata

    i = 0
    src_n = len(source)
    tgt_n = len(target)
    while i < tgt_n:
        if i < src_n and source[i] == target[i]:
            j = i + 1
            while j < tgt_n and j < src_n and source[j] == target[j]:
                j += 1
            body += encode_action(SOURCE_READ, j - i)
            i = j
        else:
            j = i + 1
            while j < tgt_n and not (j < src_n and source[j] == target[j]):
                j += 1
            body += encode_action(TARGET_READ, j - i)
            body += target[i:j]
            i = j

    body += crc32(source).to_bytes(4, "little")
    body += crc32(target).to_bytes(4, "little")
    patch = bytes(body) + crc32(body).to_bytes(4, "little")
    return patch


def apply_bps(patch: bytes, source: bytes) -> bytes:
    if len(patch) < 17 or patch[:4] != BPS_MAGIC:
        raise ValueError("not a BPS1 patch")
    if crc32(patch[:-4]) != int.from_bytes(patch[-4:], "little"):
        raise ValueError("BPS patch checksum mismatch")

    buf = io.BytesIO(patch[4:-12])
    src_size = decode_number(buf)
    tgt_size = decode_number(buf)
    meta_size = decode_number(buf)
    buf.read(meta_size)
    if src_size != len(source):
        raise ValueError(f"source size {len(source)} != patch {src_size}")
    stored_src = int.from_bytes(patch[-12:-8], "little")
    stored_tgt = int.from_bytes(patch[-8:-4], "little")
    if crc32(source) != stored_src:
        raise ValueError("source CRC does not match the patch (wrong ROM?)")

    target = bytearray()
    data_end = buf.getbuffer().nbytes
    while len(target) < tgt_size:
        if buf.tell() >= data_end:
            raise ValueError("truncated BPS actions")
        action = decode_number(buf)
        command = action & 3
        length = (action >> 2) + 1
        if command == SOURCE_READ:
            offset = len(target)
            chunk = source[offset : offset + length]
            if len(chunk) != length:
                raise ValueError("SOURCE_READ past end of source")
            target += chunk
        elif command == TARGET_READ:
            chunk = buf.read(length)
            if len(chunk) != length:
                raise ValueError("truncated TARGET_READ")
            target += chunk
        else:
            raise ValueError(f"unsupported BPS command {command}")

    target = bytes(target[:tgt_size])
    if len(target) != tgt_size:
        raise ValueError(f"patched size {len(target)} != {tgt_size}")
    if crc32(target) != stored_tgt:
        raise ValueError("target CRC mismatch after apply")
    return target


def sha1_file(path: str) -> str:
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def cmd_create(args: argparse.Namespace) -> int:
    with open(args.source, "rb") as f:
        source = f.read()
    with open(args.target, "rb") as f:
        target = f.read()
    if args.expect_vanilla:
        digest = hashlib.sha1(source).hexdigest()
        if len(source) != VANILLA_FIRERED_USA_REV0_SIZE or digest != VANILLA_FIRERED_USA_REV0_SHA1:
            print(
                "source is not FireRed USA Rev 0 "
                f"(size {len(source)}, sha1 {digest}).\n"
                f"expected size {VANILLA_FIRERED_USA_REV0_SIZE}, "
                f"sha1 {VANILLA_FIRERED_USA_REV0_SHA1}",
                file=sys.stderr,
            )
            return 1
    patch = create_bps(source, target)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)) or ".", exist_ok=True)
    with open(args.output, "wb") as f:
        f.write(patch)
    print(f"wrote {args.output} ({len(patch)} bytes)")
    return 0


def cmd_apply(args: argparse.Namespace) -> int:
    with open(args.patch, "rb") as f:
        patch = f.read()
    with open(args.source, "rb") as f:
        source = f.read()
    target = apply_bps(patch, source)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)) or ".", exist_ok=True)
    with open(args.output, "wb") as f:
        f.write(target)
    print(f"wrote {args.output} ({len(target)} bytes)")
    return 0


def cmd_selftest(_args: argparse.Namespace) -> int:
    rng = os.urandom
    source = rng(4096)
    target = bytearray(source)
    target[100:180] = rng(80)
    target += rng(256)
    patch = create_bps(bytes(source), bytes(target), metadata=b"mf-test")
    out = apply_bps(patch, bytes(source))
    if out != bytes(target):
        print("self-test: patched bytes mismatch", file=sys.stderr)
        return 1
    with tempfile.TemporaryDirectory() as tmp:
        s = os.path.join(tmp, "s.bin")
        t = os.path.join(tmp, "t.bin")
        p = os.path.join(tmp, "p.bps")
        o = os.path.join(tmp, "o.bin")
        open(s, "wb").write(source)
        open(t, "wb").write(target)
        ns = argparse.Namespace(source=s, target=t, output=p, expect_vanilla=False)
        if cmd_create(ns) != 0:
            return 1
        ns = argparse.Namespace(patch=p, source=s, output=o)
        if cmd_apply(ns) != 0:
            return 1
        if open(o, "rb").read() != bytes(target):
            print("self-test: file round-trip mismatch", file=sys.stderr)
            return 1
    print("bps self-test ok")
    return 0


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("create", help="write a BPS patch (never a ROM)")
    c.add_argument("--source", required=True, help="vanilla FireRed .gba you own")
    c.add_argument("--target", required=True, help="pokefirered-release.gba")
    c.add_argument("--output", required=True, help="path to .bps")
    c.add_argument(
        "--expect-vanilla",
        action="store_true",
        help="require FireRed USA Rev 0 SHA-1",
    )
    c.set_defaults(func=cmd_create)
    a = sub.add_parser("apply", help="apply a BPS patch to a vanilla ROM")
    a.add_argument("--patch", required=True)
    a.add_argument("--source", required=True, help="vanilla FireRed .gba you own")
    a.add_argument("--output", required=True, help="patched .gba (do not redistribute)")
    a.set_defaults(func=cmd_apply)
    t = sub.add_parser("self-test", help="round-trip synthetic files")
    t.set_defaults(func=cmd_selftest)
    args = p.parse_args()
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
