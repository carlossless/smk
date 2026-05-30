"""Drive the patched uCsim 8051 simulator to exercise SMK's USB firmware.

Requires the patched simulator (flake package `ucsim-sh68f90`; `ucsim_51` on PATH
in `nix develop`) and a built firmware `.hex`. The patch registers the SH68F90
USB interrupt (vector 7) plus a minimal SIE model that logs each EP0 IN packet
the firmware produces -- this module injects control SETUP packets and parses
that output. See tools/ucsim/ for the patch and a shell reproducer.
"""

import os
import re
import shutil
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def find_sim():
    # The patched simulator installs as `ucsim-sh68f90` (distinct from sdcc's
    # stock, unpatched `ucsim_51`).
    env = os.environ.get("SMK_UCSIM")
    if env:
        return env
    found = shutil.which("ucsim-sh68f90")
    if found:
        return found
    local = REPO_ROOT / "result" / "bin" / "ucsim-sh68f90"
    return str(local) if local.exists() else "ucsim-sh68f90"


def find_firmware():
    env = os.environ.get("SMK_FIRMWARE")
    if env and Path(env).exists():
        return env
    # cwd covers `meson test` (run from the build dir); REPO_ROOT/build covers
    # manual runs from the repo root.
    candidates = (sorted(Path.cwd().glob("*_smk.hex"))
                  + sorted(REPO_ROOT.glob("build/*_smk.hex"))
                  + sorted(REPO_ROOT.glob("*_smk.hex")))
    for c in candidates:  # prefer nuphy-air60 (the reference target)
        if "nuphy" in c.name:
            return str(c)
    return str(candidates[0]) if candidates else None


def get_descriptor(desc_type, index=0, length=64):
    """Build an 8-byte GET_DESCRIPTOR control-IN SETUP packet."""
    return [0x80, 0x06, index, desc_type, 0x00, 0x00, length & 0xFF, (length >> 8) & 0xFF]


DESC_DEVICE = 0x01
DESC_CONFIGURATION = 0x02
DESC_STRING = 0x03

# HID class request / report constants (src/smk/usbdef.h)
REPORT_ID_ISP = 0x05
REPORT_TYPE_FEATURE = 0x03
REPORT_TYPE_OUTPUT = 0x02


# --- SETUP packet builders (bmRequestType, bRequest, wValueLo, wValueHi,
#     wIndexLo, wIndexHi, wLengthLo, wLengthHi) -------------------------------
def set_address(addr):
    return [0x00, 0x05, addr & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00]


def set_configuration(cfg):
    return [0x00, 0x09, cfg & 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00]


def get_configuration():
    return [0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00]


def get_status_device():
    return [0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00]


def hid_set_idle(duration):  # wValue = duration<<8 | report_id(0)
    return [0x21, 0x0A, 0x00, duration & 0xFF, 0x00, 0x00, 0x00, 0x00]


def hid_get_idle():
    return [0xA1, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00]


def hid_set_protocol(proto, iface=0):  # wValue = protocol
    return [0x21, 0x0B, proto & 0xFF, 0x00, iface & 0xFF, 0x00, 0x00, 0x00]


def hid_get_protocol(iface=0):
    return [0xA1, 0x03, 0x00, 0x00, iface & 0xFF, 0x00, 0x01, 0x00]


def set_report_output(report_id=0, length=1):  # SET_REPORT(Output) -- LED report
    return [0x21, 0x09, report_id & 0xFF, REPORT_TYPE_OUTPUT,
            0x00, 0x00, length & 0xFF, (length >> 8) & 0xFF]


def set_report_isp_setup():
    """SET_REPORT(Feature, REPORT_ID_ISP) SETUP packet -- the firmware's path into
    the ISP bootloader."""
    # bmRequestType=0x21 (OUT|Class|Interface), bRequest=0x09 (SET_REPORT),
    # wValue=0x0305 (Feature<<8 | ISP id), wIndex=1 (iface), wLength=5
    return [0x21, 0x09, REPORT_ID_ISP, REPORT_TYPE_FEATURE, 0x01, 0x00, 0x05, 0x00]


class UsbSim:
    # SH68F90 SFR addresses / bits (see src/platform/sh68f90a/sh68f90a.h)
    IE, IEN1, USBADDR, USBIF1, USBIF2, EP0CON = 0xA8, 0xA9, 0x96, 0x92, 0x93, 0x97
    SP_SFR = 0x81
    EP0_OUT_BUF = 0x1100
    EA, EUSB, SETUPIF, OEP0IF = 0x80, 0x01, 0x10, 0x10
    USB_VECTOR = 0x3B
    ISP_BOOTLOADER = 0xFF00  # isp_jump() does: clr EA; B=0xa5; A=0x5a; ljmp 0xff00
    ISP_MAGIC_ACC, ISP_MAGIC_B = 0x5A, 0xA5
    SLED_END = 0x900E  # break address at the end of the 16-byte NOP sled

    # firmware globals (xram) and code addresses -- from the nuphy-air60 .map;
    # re-derive with: grep _name build/<target>.map
    LED_STATE = 0x0061       # keyboard_state.led_state
    KB_REPORT = 0x0025       # keyboard_report (8 bytes)
    CONN_MODE = 0x0328       # user_keyboard_state.conn_mode (1 = USB)
    IFACE0_PROTOCOL = 0x02E9
    SEND_KB_REPORT = 0x0A6E  # send_keyboard_report() entry (no args)
    CONN_MODE_USB = 0x01

    # key-matrix staging for the SIE model: a "pressed" key whose row reads pull
    # low while its column is being scanned (see cl_sh68f90_sie::read in the patch)
    KEY_ENABLE = 0x1F00      # 1 = a key is held
    KEY_COL = 0x1F01         # its column (0..15)
    KEY_ROW = 0x1F02         # its row (0..4)

    # full-init enumeration (boot from reset through the real init path)
    POST_INIT = 0x0273       # main(), right after init() returns (EA=1, usb_init done)
    MAIN_LOOP = 0x2DD5       # kb_update_switches(), first call in main's while-loop
    USB_DEVICE_STATE = 0x02E6
    STATE_DEFAULT, STATE_ADDRESSED, STATE_CONFIGURED = 0, 1, 2

    def __init__(self, firmware=None, sim=None):
        self.sim = sim or find_sim()
        self.firmware = firmware or find_firmware()

    def available(self):
        """Return None if runnable, else a human reason it is not."""
        if not self.firmware or not Path(self.firmware).exists():
            return "no SMK firmware .hex (build it: meson compile -C build nuphy-air60_default_smk.hex)"
        try:
            subprocess.run([self.sim, "-v"], capture_output=True, timeout=10)
        except (OSError, subprocess.SubprocessError):
            return f"patched simulator not runnable ({self.sim}); build it: nix build .#ucsim-sh68f90"
        return None

    def run(self, commands, timeout=30):
        # -t 52: the SH68F90 is an 8052-class part (256 bytes internal RAM). The
        # base 8051 (-t 51) has only 128 bytes, so the firmware's stack (SP grows
        # to ~0x89) would corrupt -- which is exactly why booting from reset needs
        # this model.
        script = "\n".join([f'file "{self.firmware}"'] + commands + ["quit"]) + "\n"
        p = subprocess.run(
            [self.sim, "-t", "52", "-S", "in=/dev/null"],
            input=script, capture_output=True, text=True, timeout=timeout,
        )
        return p.stdout + p.stderr

    # --- stimulus ---------------------------------------------------------
    @staticmethod
    def _set_xram(addr, data):
        return f"set mem xram 0x{addr:x} " + " ".join(f"0x{b:02x}" for b in data)

    def fire_vector(self):
        """Boot through real init, then raise USBIF1.SETUPIF and break at the USB
        vector. Captures SP before and after the vector accepts the IRQ so the
        push of the return address can be verified (post-boot SP isn't a fixed
        value, so we measure the delta rather than an absolute)."""
        cmds = self._boot_to_post_init() + [
            "echo ===PRE_IRQ===",
            f"dump sfr 0x{self.SP_SFR:x} 0x{self.SP_SFR:x}",
            f"set mem sfr 0x{self.USBIF1:x} 0x{self.SETUPIF:02x}",
            f"break 0x{self.USB_VECTOR:x}",
            "run",
            "echo ===POST_IRQ===",
            f"dump sfr 0x{self.SP_SFR:x} 0x{self.SP_SFR:x}",
        ]
        return self.run(cmds)

    def _ctrl(self, setup):
        """Commands for one control transfer: stage the SETUP, request the
        interrupt, reset PC to the sled top, and run to the sled break."""
        assert len(setup) == 8
        return [
            self._set_xram(self.EP0_OUT_BUF, setup),
            f"set mem sfr 0x{self.USBIF1:x} 0x{self.SETUPIF:02x}",
            "pc 0x9000",
            f"break 0x{self.SLED_END:x}",
            "run",
        ]

    def control_in(self, setup):
        """Boot through real init, then run a single control transfer; the SIE
        logs the EP0 IN packets."""
        return self.run(self._boot_to_post_init() + self._ctrl(setup))

    def controls(self, *setups):
        """Boot through real init, then run several control transfers in one
        session (state persists across them)."""
        cmds = self._boot_to_post_init()
        for s in setups:
            cmds += self._ctrl(s)
        return self.run(cmds)

    def set_address(self, addr):
        """Boot, SET_ADDRESS, then read USBADDR (committed during the status stage)."""
        cmds = self._boot_to_post_init() + self._ctrl(set_address(addr)) + [
            f"dump sfr 0x{self.USBADDR:x} 0x{self.USBADDR:x}",
        ]
        return self.dumped_byte(self.run(cmds))

    def led_report(self, value):
        """Boot, then SET_REPORT(Output) + OUT data stage carrying the LED byte;
        returns the firmware's keyboard_state.led_state afterwards."""
        cmds = self._boot_to_post_init() + self._ctrl(set_report_output(length=1)) + [
            # OUT data stage -> usb_ep0_out_irq (LED branch) -> led_state = buf[0]
            self._set_xram(self.EP0_OUT_BUF, [value]),
            f"set mem sfr 0x{self.USBIF2:x} 0x{self.OEP0IF:02x}",
            "pc 0x9000",
            f"break 0x{self.SLED_END:x}",
            "run",
            f"dump xram 0x{self.LED_STATE:x} 0x{self.LED_STATE:x}",
        ]
        return self.dumped_byte(self.run(cmds))

    def _boot_to_post_init(self):
        """Boot from reset through the real init path and stop right after init()
        returns (USB fully initialized, EA set), then park on a NOP sled so
        control transfers can be injected against that real state."""
        return [
            "reset",
            f"break 0x{self.POST_INIT:x}",
            "run",
            "delete",  # clear the post-init breakpoint
            "set mem rom 0x9000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
            "pc 0x9000",
        ]

    def boot_to_main_loop(self):
        """Boot from reset all the way through init + delays + kb/rf/indicator
        setup to the main while-loop (relies on the modeled UART so the boot-time
        dprintf returns). ~6s. Returns sim output (incl. UART banner)."""
        return self.run(["reset", f"break 0x{self.MAIN_LOOP:x}", "run"], timeout=45)

    def boot_post_init_state(self):
        """Boot through real init; return usb_device_state right after init()."""
        cmds = [
            "reset",
            f"break 0x{self.POST_INIT:x}",
            "run",
            f"dump xram 0x{self.USB_DEVICE_STATE:x} 0x{self.USB_DEVICE_STATE:x}",
        ]
        return self.dumped_byte(self.run(cmds))

    def boot_enumerate(self, *setups):
        """Boot through real init, then run control transfers against the real
        initialized state. Returns sim output (read with dump_value)."""
        cmds = self._boot_to_post_init()
        for s in setups:
            cmds += self._ctrl(s)
        cmds += [
            f"dump xram 0x{self.USB_DEVICE_STATE:x} 0x{self.USB_DEVICE_STATE:x}",
            f"dump sfr 0x{self.USBADDR:x} 0x{self.USBADDR:x}",
        ]
        return self.run(cmds)

    def deepest_stack_run(self):
        """Boot through real init (the firmware's stack_paint fills the stack with
        0xAA), then drive the deepest path reachable in sim and dump the painted
        stack so the high-water can be read back. The deep path combines, on top of
        the running main loop: a real key press (PWM ISR -> matrix_scan_step ->
        matrix_task -> process_key_state -> send_keyboard_report -> EP1) AND the
        deepest USB ISR (GET_DESCRIPTOR's descriptor handler) nested over it."""
        cmds = [
            "reset",
            self._set_xram(self.KEY_ENABLE, [0x01, 0x01, 0x02]),  # hold KC_A (col1,row2)
            f"break 0x{self.MAIN_LOOP:x}",
            "run", "run", "run", "run",  # boot, then run the key path (PWM/matrix/report)
            self._set_xram(self.EP0_OUT_BUF, get_descriptor(DESC_DEVICE)),
            f"set mem sfr 0x{self.USBIF1:x} 0x{self.SETUPIF:02x}",
            "run", "run", "run",  # USB ISR nests on top of the ongoing key activity
            "echo ===STACKDUMP===",
            "dump iram 0x86 0xff",
        ]
        return self.run(cmds, timeout=60)

    @staticmethod
    def stack_highwater(output, base=0x85, sentinel=0xAA):
        """Bytes of stack used at the peak: scan the painted stack dump (after the
        ===STACKDUMP=== marker) for the highest address still overwritten."""
        seg = output.split("===STACKDUMP===")[-1]
        vals = {}
        for line in seg.splitlines():
            m = re.match(r"^0x([0-9a-fA-F]{2})\s+((?:[0-9a-fA-F]{2} )+)", line)
            if m:
                addr = int(m.group(1), 16)
                for i, b in enumerate(m.group(2).split()):
                    vals[addr + i] = int(b, 16)
        for addr in range(0xFF, base, -1):
            if vals.get(addr, sentinel) != sentinel:
                return addr - base
        return 0

    def overflow_stack(self, marker=0xAA):
        """Deliberately overflow the firmware's stack (base 0x85, 122 bytes
        0x86-0xFF) with a PUSH/SJMP loop. The push past 0xFF wraps the 8-bit SP
        to 0x00, silently corrupting register bank 0 -- the 8051 has no hardware
        stack protection. uCsim's own stack tracking flags it ("Stack overflow")
        and halts. Returns sim output."""
        cmds = [
            "reset",
            "set mem rom 0x9000 0xc0 0xe0 0x80 0xfc",  # loop: PUSH ACC; SJMP $-2
            f"set mem sfr 0xe0 0x{marker:02x}",        # ACC = marker byte
            "set mem iram 0x00 0x00",                  # clear register-bank-0 byte R0
            "set mem sfr 0x81 0x85",                   # SP = firmware stack base
            "pc 0x9000",
            "run",                                     # uCsim halts at the overflow
            "dump sfr 0x81 0x81",
            "dump iram 0x00 0x00",
        ]
        return self.run(cmds)

    def keyboard_report_6kro(self, keys):
        """Boot through real init *to the main loop* (so kb_init() has run and
        conn_mode=USB is committed -- kb_init is called from main() after init()
        returns, so POST_INIT is too early), stage a keyboard_report with these
        keycodes, then invoke send_keyboard_report() (6KRO/EP1 path); returns the
        EP1 report the SIE captures. Byte 0 (modifiers) comes from separate HID
        state and is not asserted here.

        With kb_init done, iface0_protocol=BOOT (->6KRO path), and the modeled
        SIE forces P5.5 high so conn_mode reads as USB, no manual state-poking."""
        report = [0x00, 0x00] + (list(keys) + [0] * 6)[:6]  # mods, reserved, keys[6]
        # invoke send_keyboard_report by faking a return frame on the firmware's
        # actual stack: push 0x9000 (sled) at iram[base+1, base+2], set SP=base+2.
        STACK_BASE = 0x85
        cmds = [
            "reset",
            f"break 0x{self.MAIN_LOOP:x}",
            "run",  # full boot through init() + kb_init/rf_init/indicators
            "delete",
            "set mem rom 0x9000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
            "pc 0x9000",
            self._set_xram(self.KB_REPORT, report),
            f"set mem iram 0x{STACK_BASE + 1:x} 0x00",
            f"set mem iram 0x{STACK_BASE + 2:x} 0x90",
            f"set mem sfr 0x{self.SP_SFR:x} 0x{STACK_BASE + 2:x}",
            f"pc 0x{self.SEND_KB_REPORT:x}",
            "break 0x9000",
            "run",
        ]
        return self.run(cmds, timeout=45)

    def press_key(self, col, row, enable=True, iterations=10):
        """Boot to the main loop, then stage a physical key at (col, row) in the
        SIE matrix model. This drives the *real* key path end to end: the PWM ISR
        calls matrix_scan_step(), the main loop's matrix_task() detects the change
        and calls process_key_state() -- which logs `KEY: 0x<qcode>` and (in USB
        mode) sends the HID report on EP1. No cold injection. Returns sim output
        (parse with key_events() / ep1_report())."""
        cmds = [
            "reset",
            self._set_xram(self.KEY_ENABLE,
                           [0x01 if enable else 0x00, col & 0xFF, row & 0xFF]),
            f"break 0x{self.MAIN_LOOP:x}",
        ] + ["run"] * (1 + iterations)  # 1 run boots; the rest cycle the main loop
        return self.run(cmds, timeout=60)

    def trigger_isp_jump(self, confirm=(0x05, 0x75), phase2_break=None):
        """Perform the SET_REPORT(ISP) control transfer that makes the firmware
        jump into the ISP bootloader: SETUP arms USB_EP0_STATE_ISP, then the OUT
        data stage carries the confirm bytes that gate isp_jump(). Returns sim
        output including `info registers`.

        phase2_break is where phase 2 stops: the bootloader entry (default) for
        the success case, or SLED_END for the negative case where no jump should
        happen and the ISR returns to the NOP sled."""
        if phase2_break is None:
            phase2_break = self.ISP_BOOTLOADER
        out_data = list(confirm) + [0] * (8 - len(confirm))
        cmds = self._boot_to_post_init() + [
            # phase 1: SET_REPORT(Feature, ISP) SETUP -> usb_ep0_state = ISP
            self._set_xram(self.EP0_OUT_BUF, set_report_isp_setup()),
            f"set mem sfr 0x{self.USBIF1:x} 0x{self.SETUPIF:02x}",
            f"break 0x{self.SLED_END:x}",
            "run",
            # phase 2: OUT data stage -> usb_ep0_out_irq -> isp_jump() -> 0xff00.
            # reset PC to the sled top so a non-jumping ISR return lands back on
            # the sled and reaches the break (rather than sliding past it).
            self._set_xram(self.EP0_OUT_BUF, out_data),
            f"set mem sfr 0x{self.USBIF2:x} 0x{self.OEP0IF:02x}",
            "pc 0x9000",
            f"break 0x{phase2_break:x}",
            "run",
            "info registers",
        ]
        return self.run(cmds)

    # --- parsing ----------------------------------------------------------
    @staticmethod
    def in_packets(output):
        """All EP0 IN packets captured by the SIE model, as lists of byte ints."""
        out = []
        for m in re.finditer(r"\[SIE\] EP0 IN\[\d+\] \d+ bytes:((?: [0-9a-f]{2})*)", output):
            out.append([int(x, 16) for x in m.group(1).split()])
        return out

    @classmethod
    def reassemble(cls, output):
        """Concatenate the EP0 IN packets into one byte list (the full transfer)."""
        return [b for pkt in cls.in_packets(output) for b in pkt]

    @staticmethod
    def ep1_report(output):
        """The last EP1 (keyboard) report the SIE captured, as a byte list."""
        ms = re.findall(r"\[SIE\] EP1 IN \d+ bytes:((?: [0-9a-f]{2})*)", output)
        return [int(x, 16) for x in ms[-1].split()] if ms else []

    @staticmethod
    def key_events(output):
        """Every `KEY: 0x<qcode> <label>` line process_key_state() logs, as
        (qcode, label) tuples. NOTE: the firmware's label is inverted
        (`pressed ? "UP" : "DOWN"`), so "UP" means it saw the key pressed."""
        return [(int(m.group(1), 16), m.group(2))
                for m in re.finditer(r"KEY: 0x([0-9a-fA-F]{4}) (\w+)", output)]

    @staticmethod
    def dumped_byte(output):
        """Value of the last `dump` (uCsim prints `0x<addr>   <byte> ...`)."""
        ms = re.findall(r"^0x[0-9a-fA-F]+\s+([0-9a-fA-F]{2})", output, re.M)
        return int(ms[-1], 16) if ms else None

    @staticmethod
    def dump_in_section(output, start_marker, addr, end_marker=None):
        """Last dump value of `addr` in the segment after `start_marker` (and
        before `end_marker`, if given). Lets a single sim run capture the same
        register/cell at multiple points (e.g. SP before and after an IRQ).
        Handles both uCsim dump formats: unnamed/xram (`0xADDR  HH HH ...`) and
        named SFR (`0xADDR NAME:    0b<bits> 0xHH 'c' 135 (-121)`)."""
        seg = output.split(start_marker, 1)[-1]
        if end_marker:
            seg = seg.split(end_marker, 1)[0]
        # After `0xADDR` on the line, accept either: (a) whitespace then 2 hex
        # digits (unnamed SFR / xram), or (b) anything-then-`0x` then 2 hex digits
        # (named SFR -- 0xHH appears after the binary representation).
        pat = rf"^0x0*{addr:x}\b(?:[^\n]*?\b0x|[ \t]+)([0-9a-fA-F]{{2}})\b"
        ms = re.findall(pat, seg, re.M | re.I)
        return int(ms[-1], 16) if ms else None

    @staticmethod
    def dump_value(output, addr):
        """Value of the LAST `dump` for a specific address (sfr prints 2 hex
        digits, xram 4). Last match because `set mem` echoes the same address."""
        for pat in (rf"0x0*{addr:x}\b", rf"0x{addr:04x}", rf"0x{addr:02x}"):
            ms = re.findall(rf"^{pat}\s+([0-9a-fA-F]{{2}})", output, re.M | re.I)
            if ms:
                return int(ms[-1], 16)
        return None

    @staticmethod
    def _last(pattern, output):
        # a command sequence can have several runs / register dumps; the final
        # state is the last match
        matches = re.findall(pattern, output)
        return int(matches[-1], 16) if matches else None

    @classmethod
    def stopped_at(cls, output):
        return cls._last(r"Stop at 0x([0-9a-fA-F]+)", output)

    @classmethod
    def sp(cls, output):
        return cls._last(r"\bSP 0x([0-9a-fA-F]+)", output)

    @classmethod
    def acc(cls, output):
        return cls._last(r"\bACC= 0x([0-9a-fA-F]+)", output)

    @classmethod
    def reg_b(cls, output):
        return cls._last(r"\bB= 0x([0-9a-fA-F]+)", output)
