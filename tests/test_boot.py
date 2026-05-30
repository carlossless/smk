#!/usr/bin/env python3
"""Boot/init firmware tests, driven through the patched uCsim simulator.

Run from the repo root (inside `nix develop`, after building firmware):

    meson compile -C build nuphy-air60_default_smk.hex
    python3 -m unittest discover -s tests        # or: python3 tests/test_boot.py

Override targets with env vars SMK_UCSIM (simulator) and SMK_FIRMWARE (.hex).
"""

import unittest

from usbsim import UsbSim

SIM = UsbSim()


def setUpModule():
    reason = SIM.available()
    if reason:
        raise unittest.SkipTest(reason)


class TestFullBoot(unittest.TestCase):
    """Boot from RESET all the way to the firmware's main loop -- exercising the
    complete real startup: crt0 -> main -> init (ldo/clock[PLL]/user/matrix/
    keyboard/usb) -> dprintf-over-UART -> delays -> kb/rf/indicator init -> the
    while-loop. Relies on the modeled UART (so the boot-time dprintf returns) and
    -t 52."""

    def test_boot_runs_full_init_to_main_loop(self):
        out = SIM.boot_to_main_loop()
        self.assertEqual(SIM.stopped_at(out), SIM.MAIN_LOOP,
                         "should reach the main while-loop after full init")
        self.assertIn("SMK v", out,
                      "boot banner should be emitted via the modeled UART")


if __name__ == "__main__":
    unittest.main()
