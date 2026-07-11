#!/usr/bin/env python3
"""Compile one source with SDCC while also emitting a make-style header depfile.

SDCC's -M family (-M/-MM/-MD/-MMD) is *deps-only*: it writes the dependency list
but suppresses code generation, leaving an empty .rel. GCC's -MMD does both in one
pass; SDCC can't, so we invoke it twice:

  1. `sdcc <args> -MMD -MF=<depfile>` — writes <depfile> (its target is the -o
     value, so it matches what ninja expects) and truncates the .rel to empty.
  2. `sdcc <args>` — the real compile, refilling the .rel.

Step 2's exit status is authoritative; step 1 is best-effort so a dependency-scan
hiccup never fails an otherwise-good build (worst case: that one .rel just isn't
header-tracked until it next rebuilds).

Usage: sdcc_compile.py <depfile> <sdcc> [args... -c <input> -o <output.rel>]
"""
import subprocess
import sys

depfile = sys.argv[1]
cmd = sys.argv[2:]  # sdcc + all flags, ending in -c <input> -o <output.rel>

# 1. dependency scan (also empties the .rel — that's why the real compile follows)
subprocess.run(cmd + ["-MMD", "-MF=" + depfile])

# 2. real compile — refills the .rel; this is the status the build acts on
sys.exit(subprocess.run(cmd).returncode)
