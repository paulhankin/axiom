#!/usr/bin/env python3
"""Writes src/shaders.ax from src/shaders.metal: the Metal source as an
array of lines, which the program joins and compiles at run time.

    python3 pinball/tools/embed.py
"""
import os

here = os.path.dirname(os.path.abspath(__file__))
src = os.path.join(here, "..", "src", "shaders.metal")
out = os.path.join(here, "..", "src", "shaders.ax")

lines = open(src).read().split("\n")
while lines and lines[-1] == "":
    lines.pop()


def quote(s):
    s = s.replace("\\", "\\\\").replace('"', '\\"').replace("\t", "    ")
    return '"' + s + '"'


body = ",\n".join("    " + quote(l) for l in lines)
open(out, "w").write(
    "// =============================================================================\n"
    "// File: shaders.ax\n"
    "// The shaders, from shaders.metal (written by tools/embed.py: edit that,\n"
    "// then run it).\n"
    "// =============================================================================\n\n"
    "pub let MSL: [%d][]u8 = [\n%s\n];\n" % (len(lines), body)
)
print("wrote %s (%d lines)" % (out, len(lines)))
