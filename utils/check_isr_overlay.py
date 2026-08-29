#!/usr/bin/env python3
"""Fail the build if any interrupt-reachable function shares SDCC's static overlay
with a main-reachable function.

Background (see project_sdcc_isr_overlay_collision): under --model-small SDCC packs
each non-reentrant function's locals/params into a shared "overlay" area (OSEG /
BIT_BANK) to save RAM, reusing the same internal-RAM bytes for functions it thinks
never run at once. That analysis does NOT account for interrupt preemption, so it
will happily park a USB-ISR helper's pointer on the very bytes a main-loop function
is using - and the ISR then corrupts it mid-render. The concrete failure was a hung
`__gptrput` (a stomped generic-pointer type byte → its code-space `sjmp .` trap).

This check reconstructs the call graph from the generated .asm, marks every function
reachable from an `__interrupt` handler and every function reachable from `main`,
and flags any overlay slot occupied by both. Fix a hit by making the ISR-side
function `__reentrant` (params move to the stack, out of the overlay).

Assumptions / limits:
  * Interrupts are equal-priority (no nesting), so ISR<->ISR overlay sharing is
    safe and not flagged; only ISR<->main sharing is.
  * The call graph is built from direct lcall/ljmp edges. Calls made through
    function pointers are invisible here - don't dispatch ISR work indirectly.
"""
import argparse
import glob
import re
import sys

# SDCC naked, register-only helpers: they reserve a PARM slot in the overlay but
# never read/write it, so co-locating them is harmless. Keep this list minimal and
# only for routines verified to touch no memory-resident parameter.
SAFE = {"__gptrput", "__gptrget"}

# Overlay areas that hold per-function locals/params. REG_BANK_* is saved/restored
# by the ISR prologue; SSEG is the stack - neither is an overlay-collision hazard.
OVERLAY_AREAS = {"OSEG", "BIT_BANK"}


def isr_symbols(src_root):
    """Function symbols (`_name`) of every `__interrupt` handler in the sources."""
    names = set()
    for path in glob.glob(src_root + "/**/*.c", recursive=True):
        text = open(path, errors="ignore").read()
        for m in re.finditer(r"\b(\w+)\s*\([^;{}]*\)\s*__interrupt", text):
            names.add("_" + m.group(1))
    return names


def call_graph(asm_dir):
    """func_symbol -> set(callee_symbol), from direct lcall/ljmp edges in the .asm."""
    calls = {}
    for path in glob.glob(asm_dir + "/*.asm"):
        cur = None  # reset per file: calls before the first label belong to no function
        for line in open(path, errors="ignore"):
            lbl = re.match(r"^(_[A-Za-z]\w*):", line)
            if lbl:
                cur = lbl.group(1)
                calls.setdefault(cur, set())
                continue
            edge = re.match(r"^\s+(?:l?call|ljmp)\s+(__?[A-Za-z]\w*)", line)
            if edge and cur:
                calls[cur].add(edge.group(1))
    return calls


def reachable(calls, roots):
    seen, stack = set(), list(roots)
    while stack:
        f = stack.pop()
        if f in seen:
            continue
        seen.add(f)
        stack.extend(calls.get(f, ()))
    return seen


def sym_to_func(sym):
    """Map an overlay symbol to its owning function symbol, or None.

    Forms seen in the map:
      Lmodule.func$var$scope   (locals; module prefix can be truncated at ~32 cols)
      _func_PARM_n             (parameters)
      _func_localname_NNNN     (locals)
      __libfn_PARM_n           (library-routine parameters)
    """
    m = re.match(r"^L\w+?\.([A-Za-z_]\w*)\$", sym)
    if m:
        return "_" + m.group(1)
    m = re.match(r"^(__?[A-Za-z]\w*?)_(?:PARM_\d+|[A-Za-z]\w*?_\d{3,})$", sym)
    if m:
        return m.group(1)
    return None


def overlay_slots(map_file):
    """address -> set(function symbols) for the overlay areas.

    Every overlay entry appears under two symbols: a module-qualified `Lmod.func$var`
    form and a short `_func_...` form. The linker truncates the "Global" column at ~32
    chars, which can cut the function name out of the long L-form - but the short form
    (no module prefix) resolves, so a truncated L-form is always redundant and dropped
    silently. Anything else unresolved is a genuine blind spot and is returned.
    """
    slots = {}
    unresolved = []
    area = None
    for line in open(map_file, errors="ignore"):
        head = re.match(r"^(\w+)\s+[0-9A-Fa-f]{8}\s+[0-9A-Fa-f]{8}", line)
        if head:
            area = head.group(1)
            continue
        row = re.match(r"^\s+([0-9A-Fa-f]{8})\s+(\S+)", line)
        if not row or area not in OVERLAY_AREAS:
            continue
        addr, sym = row.group(1).lower(), row.group(2)
        func = sym_to_func(sym)
        if func is None:
            if not sym.startswith("L"):  # L-forms are redundant with their _-form
                unresolved.append((addr, sym))
            continue
        slots.setdefault(addr, set()).add(func)
    return slots, unresolved


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", required=True, help="source root to scan for __interrupt handlers")
    ap.add_argument("--asmdir", required=True, help="dir with the board's generated .asm files")
    ap.add_argument("--map", required=True, help="the board's linker .map file")
    ap.add_argument("--stamp", help="touch this file on success (for ninja)")
    args = ap.parse_args()

    isr_roots = isr_symbols(args.src)
    if not isr_roots:
        print("check_isr_overlay: found no __interrupt handlers — refusing to pass "
              "a check that can't see any ISR (wrong --src?)", file=sys.stderr)
        return 2

    calls = call_graph(args.asmdir)
    if not calls:
        print(f"check_isr_overlay: no .asm files under {args.asmdir}", file=sys.stderr)
        return 2

    isr_reach = reachable(calls, isr_roots)
    main_reach = reachable(calls, {"_main"})
    slots, unresolved = overlay_slots(args.map)

    collisions = []
    for addr, funcs in sorted(slots.items()):
        isr_f = sorted(f for f in funcs if f in isr_reach and f not in SAFE)
        main_f = sorted(f for f in funcs if f in main_reach and f not in SAFE)
        if isr_f and main_f:
            collisions.append((addr, isr_f, sorted(set(main_f) - set(isr_f))))

    if collisions:
        print("check_isr_overlay: FAIL — interrupt-reachable functions share SDCC's "
              "overlay with the main loop (an ISR can corrupt main-loop state):",
              file=sys.stderr)
        for addr, isr_f, main_f in collisions:
            print(f"  @0x{addr[-2:]}  ISR-side {isr_f}", file=sys.stderr)
            print(f"          collides with main-side {main_f[:8]}"
                  f"{' …' if len(main_f) > 8 else ''}", file=sys.stderr)
        print("  Fix: mark the ISR-side function(s) __reentrant (see usb.c EP0 helpers).",
              file=sys.stderr)
        return 1

    if unresolved:
        # Don't fail, but surface blind spots so truncation can't silently hide a hit.
        print(f"check_isr_overlay: note — {len(unresolved)} overlay symbol(s) could not "
              f"be attributed to a function (e.g. {unresolved[0][1]}); not treated as a "
              f"collision.", file=sys.stderr)

    print(f"check_isr_overlay: OK — {len(isr_reach)} ISR-reachable / {len(main_reach)} "
          f"main-reachable functions, no overlay collisions.")
    if args.stamp:
        open(args.stamp, "w").close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
