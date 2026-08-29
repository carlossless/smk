#!/usr/bin/env python3
"""Source-level breakpoint tests, driven through the patched uCsim simulator.

These exercise the .cdb-based source mapping (tests/sim.py: load_lines /
addr_of_line / break_line): a source `file:line` resolves to a code address, so
tests can break on a specific line of C -- not just a function entry from the
.map. The mapping comes from SDCC's `.cdb` (emitted by the debug build), so
these tests skip when only a non-debug firmware is present.

Run from the repo root (inside `nix develop`, after building firmware):

    meson compile -C build nuphy-air60_default_smk.hex
    python3 -m unittest discover -s tests        # or: python3 tests/test_source_breakpoints.py
"""

import unittest

from sim import Sim, REPO_ROOT

SIM = Sim()


def main_c_line(substr):
    """1-based line number of the first main.c line containing substr. Derived
    from source so these tests don't hardcode line numbers that drift as main.c
    is edited."""
    path = REPO_ROOT / "src" / "main.c"
    for i, line in enumerate(path.read_text().splitlines(), 1):
        if substr in line:
            return i
    raise AssertionError(f"{substr!r} not found in main.c")


def codeless_main_c_line():
    """A main.c line number with no code of its own but a later line that has it,
    so it must resolve forward -- the first gap inside the mapped range."""
    present = sorted(ln for (f, ln) in SIM.lines if f == "main.c")
    for ln in range(present[0], present[-1]):
        if ln not in present:
            return ln
    raise AssertionError("no codeless line found in main.c's mapped range")


def setUpModule():
    reason = SIM.available()
    if reason:
        raise unittest.SkipTest(reason)
    if not SIM.has_lines():
        raise unittest.SkipTest(
            "no .cdb source-line map (build with debug info: "
            "meson configure build -Dbuildtype=debug && meson compile -C build)"
        )


class TestSourceLevelBreakpoints(unittest.TestCase):
    def test_line_map_loaded_and_broad(self):
        """The .cdb maps lines across the whole firmware, not just main.c."""
        files = {f for (f, _ln) in SIM.lines}
        self.assertIn("main.c", files)
        self.assertIn("matrix.c", files)
        self.assertGreaterEqual(len(files), 10,
                                "expected source lines from many modules")

    def test_break_at_source_line_halts_there(self):
        """A `file:line` breakpoint stops the CPU at exactly the address the .cdb
        assigns to that line -- the core proof that source-level breakpoints work
        and that the .cdb addresses line up with the running .hex."""
        # watchdog_kick() is the first statement of the main while-loop, reached
        # after the full boot/init path.
        line = main_c_line("watchdog_kick();")
        addr = SIM.addr_of_line("main.c", line)
        out = SIM.run(["reset", SIM.break_line("main.c", line), "run"], timeout=30)
        self.assertEqual(SIM.stopped_at(out), addr,
                         f"execution should halt at the main.c:{line} breakpoint")

    def test_nearest_line_adjustment(self):
        """Lines without their own code (folded into a neighbour) resolve forward
        to the next line that does, gdb-style, and report where they landed."""
        line = codeless_main_c_line()
        landed = SIM.actual_line("main.c", line)
        self.assertGreater(landed, line, f"{line} has no code; should move forward")
        self.assertEqual(SIM.addr_of_line("main.c", line),
                         SIM.addr_of_line("main.c", landed),
                         "the adjusted line and the request resolve to one address")

    def test_exact_mode_rejects_codeless_line(self):
        """exact=True does not silently move the breakpoint."""
        with self.assertRaises(KeyError):
            SIM.addr_of_line("main.c", codeless_main_c_line(), exact=True)


if __name__ == "__main__":
    unittest.main()
