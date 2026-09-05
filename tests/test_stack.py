#!/usr/bin/env python3
"""Stack-usage firmware tests, driven through the patched uCsim simulator.

The SH68F90 is an 8052-class part with 256 bytes of internal RAM; the firmware
reserves 122 bytes of it (0x86-0xFF, base 0x85) as the call stack. The 8051 has
no hardware overflow detection -- SP wraps 0xFF->0x00 and silently corrupts
register bank 0 / the static data area. These tests measure the deepest
reachable path and exercise the overflow behaviour as a tool for catching real
overflows.

Run from the repo root (inside `nix develop`, after building firmware):

    meson setup build --cross-file cross-file/nuphy-air60.ini
    meson compile -C build
    python3 -m unittest discover -s tests        # or: python3 tests/test_stack.py

Override targets with env vars SMK_UCSIM (simulator) and SMK_FIRMWARE (.hex).
"""

import unittest

from sim import Sim
from devices import Air60Sim

SIM = Sim()


def setUpModule():
    reason = SIM.available()
    if reason:
        raise unittest.SkipTest(reason)


class TestWorstCaseStack(unittest.TestCase):
    """Drive the deepest stack path reachable in simulation -- a real key press
    (matrix scan -> process_key_state -> EP1 report) with the deepest USB ISR
    (GET_DESCRIPTOR's descriptor handler) nested over it -- and verify the firmware
    reaches the end without overflowing the 122-byte stack. The peak is read back
    from the boot-time painted region; uCsim's stack tracking is the backstop.

    Caveat: the deepest key-path and USB-ISR frames being live at the *same
    instant* isn't forced (the paint captures the deepest SP ever reached), so this
    is the deepest *reachable* combination, not a provable global maximum."""

    def test_deepest_path_reaches_end_without_overflow(self):
        kb = Air60Sim()
        try:
            kb.boot(usb=True)                 # stack_paint runs during boot
            kb.mark_usb_configured()          # enumerated host: usb_send_* gates on CONFIGURED
            kb.matrix.press(1, 2)             # KC_A: real scan -> process_key_state -> EP1
            reps = kb.scan_for_report()
            kb.nest_usb_descriptor()          # deepest USB ISR nested over the main loop
            peak = kb.stack_highwater()
            stack_base = kb.stack_base
            serr = kb.stderr_text()
        finally:
            kb.close()
        self.assertNotIn("Stack overflow", serr,
                         "firmware must not overflow its stack on the deepest path")
        self.assertTrue(reps and reps[-1][2] == 0x04,
                        f"the real key path (KC_A) must have run; EP1 reports: {reps}")
        self.assertIn("EP0 IN", serr, "the deep USB ISR must have run to completion")
        cap = Air60Sim.STACK_TOP - stack_base   # usable stack bytes for this build
        self.assertGreater(peak, 0, "stack high-water should be measurable")
        self.assertLess(peak, cap, f"stack peak {peak} must stay within {cap} bytes")
        print(f"\n[worst-case stack] deepest reached = {peak}/{cap} bytes")


class TestStackOverflow(unittest.TestCase):
    """Force a stack overflow and verify the SP wraps 0xFF->0x00, corrupting
    register bank 0, and that uCsim's stack tracking catches it (a tool for
    spotting real overflows)."""

    def test_overflow_wraps_sp_and_corrupts_register_bank(self):
        out = SIM.overflow_stack(marker=0xAA)
        self.assertIn("Stack overflow", out, "uCsim should detect the overflow")
        # the push past 0xFF wrapped SP to 0x00 and overwrote register-bank-0 R0
        self.assertEqual(SIM.dump_value(out, 0x00), 0xAA,
                         "overflow must clobber register-bank-0 byte at 0x00")


if __name__ == "__main__":
    unittest.main()
