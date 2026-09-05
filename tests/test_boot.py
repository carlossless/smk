#!/usr/bin/env python3
"""Boot/init firmware tests, driven through the patched uCsim simulator.

Run from the repo root (inside `nix develop`, after building firmware):

    meson setup build --cross-file cross-file/nuphy-air60.ini
    meson compile -C build
    python3 -m unittest discover -s tests        # or: python3 tests/test_boot.py

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


class TestFullBoot(unittest.TestCase):
    """Boot from RESET through the complete real startup to the main while-loop
    and confirm the boot banner is emitted. The banner routes to the console ring
    buffer (drained over USB EP2 on hardware), so finding it there proves the
    whole init chain ran."""

    def test_boot_runs_full_init_to_main_loop(self):
        kb = Air60Sim()
        try:
            kb.boot(usb=True)
            banner = kb.console_text()
        finally:
            kb.close()
        self.assertIn("SMK v", banner,
                      f"boot banner should reach the console ring buffer; got {banner!r}")


if __name__ == "__main__":
    unittest.main()
