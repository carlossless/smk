#!/usr/bin/env python3
"""Stack-usage firmware tests, driven through the patched uCsim simulator.

The SH68F90A is an 8052-class part with 256 bytes of internal RAM; the firmware
reserves 122 bytes of it (0x86-0xFF, base 0x85) as the call stack. The 8051 has
no hardware overflow detection -- SP wraps 0xFF->0x00 and silently corrupts
register bank 0 / the static data area. These tests measure the deepest
reachable path and exercise the overflow behaviour as a tool for catching real
overflows.

Run from the repo root (inside `nix develop`, after building firmware):

    meson compile -C build nuphy-air60_default_smk.hex
    python3 -m unittest discover -s tests        # or: python3 tests/test_stack.py

Override targets with env vars SMK_UCSIM (simulator) and SMK_FIRMWARE (.hex).
"""

import unittest

from sim import Sim

SIM = Sim()


def setUpModule():
    reason = SIM.available()
    if reason:
        raise unittest.SkipTest(reason)


class TestWorstCaseStack(unittest.TestCase):
    """Drive the deepest stack path reachable in simulation -- a full boot through
    real init, then a real key press (PWM ISR -> matrix scan -> process_key_state
    -> EP1 report) with the deepest USB ISR (GET_DESCRIPTOR's descriptor handler)
    nested over it -- and verify the firmware reaches the end without overflowing
    the 122-byte stack. The firmware paints the stack at boot (stack_paint), so the
    peak is read back from the painted region; uCsim's stack tracking is the
    overflow backstop.

    Caveat: the deepest frames of the key path and the USB ISR being live at the
    *same instant* isn't forced (the paint captures the deepest SP ever reached, not
    proven simultaneity), so this is the deepest *reachable* combination, not a
    provable global maximum."""

    def test_deepest_path_reaches_end_without_overflow(self):
        out = SIM.deepest_stack_run()
        self.assertNotIn("Stack overflow", out,
                         "firmware must not overflow its stack on the deepest path")
        self.assertIn(0x0004, [q for q, _ in SIM.key_events(out)],
                      "the real key path (KC_A) must have run during the measurement")
        self.assertTrue(SIM.in_packets(out),
                        "the deep USB ISR must have run to completion")
        peak = SIM.stack_highwater(out)
        self.assertGreater(peak, 0, "stack high-water should be measurable")
        self.assertLess(peak, 122, f"stack peak {peak} must stay within 122 bytes")
        print(f"\n[worst-case stack] deepest reached = {peak}/122 bytes")


class TestStackOverflow(unittest.TestCase):
    """The 8051 has no hardware stack-overflow protection: the 8-bit SP simply
    wraps 0xFF->0x00 and silently corrupts low RAM / register bank 0. The
    firmware's usable stack is 122 bytes (0x86-0xFF, base 0x85). This forces an
    overflow and verifies the corruption -- and that uCsim's stack tracking
    catches it (a tool for spotting real overflows)."""

    def test_overflow_wraps_sp_and_corrupts_register_bank(self):
        out = SIM.overflow_stack(marker=0xAA)
        self.assertIn("Stack overflow", out, "uCsim should detect the overflow")
        # the push past 0xFF wrapped SP to 0x00 and overwrote register-bank-0 R0
        self.assertEqual(SIM.dump_value(out, 0x00), 0xAA,
                         "overflow must clobber register-bank-0 byte at 0x00")


if __name__ == "__main__":
    unittest.main()
