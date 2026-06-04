#!/usr/bin/env python3
"""Boot/init firmware tests, driven through the patched uCsim simulator.

Run from the repo root (inside `nix develop`, after building firmware):

    meson compile -C build nuphy-air60_default_smk.hex
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
    """Boot from RESET through the complete real startup -- crt0 -> main ->
    init (ldo/clock[PLL]/user/matrix/keyboard/usb) -> kb/rf/indicator init -> the
    main while-loop -- and confirm the boot banner is emitted. dprint_str()
    routes to the console ring buffer (drained over USB EP2 on hardware), so the
    banner (printed in main() after init() returns) is read back from there;
    finding it proves the whole init chain ran. The board's CONN_MODE switch and
    key matrix are emulated test-side, keeping the simulator a pure SH68F90."""

    def test_boot_runs_full_init_to_main_loop(self):
        kb = Air60Sim()
        try:
            kb.boot(usb=True)            # runs full init through to the main loop
            banner = kb.console_text()
        finally:
            kb.close()
        self.assertIn("SMK v", banner,
                      f"boot banner should reach the console ring buffer; got {banner!r}")


if __name__ == "__main__":
    unittest.main()
