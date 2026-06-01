# -*- coding: utf-8 -*-
# python3
# 
# usage:
#       python3 patch_start_main_default.py libc.so.6
#
import subprocess
import sys
import re
import struct

path = sys.argv[1]

out = subprocess.check_output(
    ["readelf", "-Ws", "--wide", path],
    text=True
)

targets = {}

for line in out.splitlines():
    if "__libc_start_main" not in line:
        continue

    m = re.match(r"\s*(\d+):.*(__libc_start_main)(@@?)(GLIBC_[0-9.]+)", line)
    if not m:
        continue

    idx = int(m.group(1))
    ver = m.group(4)
    targets[ver] = idx

print(targets)

old_idx = targets["GLIBC_2.2.5"]
new_idx = targets["GLIBC_2.34"]

sec = subprocess.check_output(
    ["readelf", "-S", "--wide", path],
    text=True
)

gnu_version_offset = None

for line in sec.splitlines():
    if ".gnu.version" in line and "VERSYM" in line:
        parts = line.split()
        # readelf -S --wide:
        # [ 6] .gnu.version VERSYM addr offset size ...
        gnu_version_offset = int(parts[5], 16)
        break

if gnu_version_offset is None:
    raise RuntimeError("cannot find .gnu.version")

with open(path, "r+b") as f:
    def read_half(sym_idx):
        f.seek(gnu_version_offset + sym_idx * 2)
        return struct.unpack("<H", f.read(2))[0]

    def write_half(sym_idx, val):
        f.seek(gnu_version_offset + sym_idx * 2)
        f.write(struct.pack("<H", val))

    old_val = read_half(old_idx)
    new_val = read_half(new_idx)

    print("before:")
    print("GLIBC_2.2.5:", hex(old_val))
    print("GLIBC_2.34 :", hex(new_val))

    # 0x8000 是 hidden bit
    # 默认版本：不带 0x8000
    # 非默认版本：带 0x8000
    write_half(old_idx, old_val & ~0x8000)
    write_half(new_idx, new_val | 0x8000)

    print("patched")