#!/usr/bin/env python3
"""Guard two interrupt invariants over the linked firmware.

1. Every vector in `enum interrupt_index` is claimed by a handler. SDCC pads unclaimed
   slots below the highest declared one with `reti` and emits nothing past it, so an
   unclaimed source either sticks (its flag never cleared) or runs whatever the linker
   parked after the table.

2. No interrupt-reachable function shares SDCC's static overlay with a main-reachable one
   (project_sdcc_isr_overlay_collision). SDCC packs non-reentrant locals/params into a
   shared area for functions it thinks never run at once, an analysis that ignores
   interrupt preemption - so an ISR can stomp main-loop state. It showed up as a hung
   `__gptrput`. Fix a hit by making the ISR-side function `__reentrant`.

Handlers are read off the emitted vector table, so one defined through a macro counts
like any other.

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

# Shares the table, but jumps to main's startup rather than to a handler.
RESET_VECTOR = 0
VECTOR_STRIDE = 8


def declared_vectors(src_root):
    """Vector index -> name, from `enum interrupt_index` in the platform header."""
    for path in glob.glob(src_root + "/**/*.h", recursive=True):
        text = open(path, errors="ignore").read()
        body = re.search(r"enum\s+interrupt_index\s*\{(.*?)\}", text, re.S)
        if body:
            return {int(num): name for name, num in re.findall(r"(\w+)\s*=\s*(\d+)", body.group(1))}
    return {}


def vector_table(asm_dir):
    """Vector index -> handler symbol, by walking the emitted bytes of the table."""
    for path in glob.glob(asm_dir + "/*.asm"):
        lines = open(path, errors="ignore").read().splitlines()
        if "__interrupt_vect:" not in lines:
            continue
        handlers, offset = {}, 0
        for line in lines[lines.index("__interrupt_vect:") + 1:]:
            jump = re.match(r"^\s+ljmp\s+(\S+)", line)
            pad = re.match(r"^\s+\.ds\s+(\d+)", line)
            if jump:
                handlers[offset // VECTOR_STRIDE] = jump.group(1)
                offset += 3
            elif pad:
                offset += int(pad.group(1))
            elif re.match(r"^\s+reti\b", line):
                offset += 1
            else:
                break
        return handlers
    return {}


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
    ap.add_argument("--src", required=True, help="source root holding enum interrupt_index")
    ap.add_argument("--asmdir", required=True, help="dir with the board's generated .asm files")
    ap.add_argument("--map", required=True, help="the board's linker .map file")
    ap.add_argument("--stamp", help="touch this file on success (for ninja)")
    args = ap.parse_args()

    vectors = declared_vectors(args.src)
    if not vectors:
        print("check_interrupts: no `enum interrupt_index` under --src — refusing to pass "
              "a check that can't see the part's vector list", file=sys.stderr)
        return 2

    handlers = vector_table(args.asmdir)
    if not handlers:
        print(f"check_interrupts: no interrupt vector table in the .asm under {args.asmdir}",
              file=sys.stderr)
        return 2

    unclaimed = sorted(v for v in vectors if v not in handlers)
    if unclaimed:
        print("check_interrupts: FAIL — vectors with no handler (the source can only "
              "stick or run unrelated code if it ever fires):", file=sys.stderr)
        for v in unclaimed:
            print(f"  {v:2d}  {vectors[v]}", file=sys.stderr)
        print("  Fix: declare a handler in interrupts.h — an unused one belongs in "
              "UNUSED_INTERRUPTS().", file=sys.stderr)
        return 1

    isr_roots = {sym for v, sym in handlers.items() if v != RESET_VECTOR}
    calls = call_graph(args.asmdir)
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
        print("check_interrupts: FAIL — interrupt-reachable functions share SDCC's "
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
        print(f"check_interrupts: note — {len(unresolved)} overlay symbol(s) could not "
              f"be attributed to a function (e.g. {unresolved[0][1]}); not treated as a "
              f"collision.", file=sys.stderr)

    print(f"check_interrupts: OK — {len(vectors)} vectors claimed, {len(isr_reach)} "
          f"ISR-reachable / {len(main_reach)} main-reachable functions, no overlay collisions.")
    if args.stamp:
        open(args.stamp, "w").close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
