# SH68F90A Sleep — stock air60 state machine + SMK implementation

Full reverse-engineering of how the **stock NuPhy Air60 firmware** (`air60.bin`)
puts the SH68F90A to sleep, and how SMK's `sleep` feature maps onto it. All stock
addresses are CODE-space in `nuphy-rev-eng/ghidra` (`air60.bin`). SFR names are the
real SH68F90A ones (Ghidra's generic-8051 labels are wrong — everything below is
decoded from raw opcodes against `src/platform/sh68f90a/sh68f90a.h`).

> Provenance: traced function-by-function from the binary. The **mechanism** below
> is solid (decompiled + opcode-checked). The two items called out as *approximate*
> (the SUSPIF re-fire cadence and the exact configurable RF threshold value) depend
> on runtime/hardware behaviour that the static image doesn't fully pin down.

## 1. Power-Down is real `PCON.PD`, in two variants

Stock never uses Idle mode; both sleep paths are SH68F90A **Power-Down**
(`SUSLO=0x55; PCON|=0x02`). Which variant runs is chosen by the conn slider:

| | RF / battery variant | USB variant |
|---|---|---|
| stock entry | `0x7A7D` → `0x7B48` | `0x8DFD` |
| regulator | `REGCON &= ~REGEN` (USB LDO **off** — battery win) | regulator **stays on** |
| USB module | `USBCON &= 0x3F/0x3B` (disabled) | `USBCON |= GOSUSP` (suspended) |
| `IEN1` during PD | `0` (INT4 is the only wake) | `EUSB` (USB bus events also wake) |
| wake re-init | `REGCON|=REGEN`; 500µs; **full `usb_init`** (`0x001E`: `USBADDR=0`, re-arm, re-enumerate) | **resume only** — `USBIE1=0x5F; IEN1|=EUSB`, no re-init (stays enumerated) |
| GPIO park | identical per-port block (`0x8E1A…`) | identical |
| clock teardown/rebuild | identical | identical |

Both share the per-port GPIO park (every `PxCR`/`PxPCR`/`Px` set to a low-power
state) and the clock dance: tear down `CLKCON.FS/HFON` + `PLLCON.PLLFS/PLLON`
before PD; on wake re-open the PLL.

### Wake clock rebuild
Stock wake: `LCALL 0xADC4` (enable `HFON`+`PLLON` then a **fixed NOP warm-up
spin**, no `PLLSTA` poll) → `PLLCON=0x03` (set `PLLFS`) → `CLKCON=0x0C` (set `FS`).
The PLL must be warmed/locked **before** `PLLFS` selects its output, or the CPU
runs off an unlocked PLL (system clock crawls; LED scan visibly slow).

## 2. Wake sources

- **INT4** (vector 1 → ISR `0x01FA`): the keypress wake. ISR is tiny —
  clears its flag and sets `_c_3` (bit `0x63`), the "remote-wakeup requested"
  flag. INT4 covers both the keypress wake pin **P4.1** and the BK3632 ACK line
  **P4.2** (both are INT4 sources; `EX4` armed in the GPIO-park block).
- **USB bus event** (USB variant only — `EUSB` left enabled): a host resume also
  wakes the core.
- On wake, if `_c_3` is set the wake path raises `USBCON.WKUP` to signal a USB
  remote-wakeup resume to the host.

## 3. The trigger state machine (this is the part that's intricate)

There are **two independent inactivity timers**, one per mode, plus the slider
mode flag. The main loop (`0x779A`) reads them and enters the matching variant.

### Mode flag `[0x039F]`
`stock_slider_debounce` (`0x96CE`) debounces the conn slider (P5.5) and commits
**`[0x039F]`: `0` = USB, `1` = RF** (P5.5 `1`=USB→counter `0x0BB3`, `0`=RF→`0x0BB4`;
the winner sets the flag).

### USB timer — `[0x0BEF:0x0BF0]`, ticked by USB SUSPEND
- Ticked + threshold-checked by `FUN_b2f6` (`0xB2F6`), whose **only** entry is a
  tail-`LJMP 0xB2F6` at `0x9C6A`, inside the **USB suspend (`SUSPIF`) handler**
  (`0x9C1E`). i.e. the counter advances whenever the USB bus is idle (`SUSPIF`
  = no SOF). An actively-polling host (SOF every 1 ms) → no `SUSPIF` → no tick →
  never sleeps. A suspended host *or* battery/no-host → it ticks.
- At threshold **`0xC8` (200)** it sets `_b_2` (bit `0x5A`), the USB sleep request.
- *(Approximate: how fast `SUSPIF` re-asserts while the bus stays idle — i.e. the
  real-world USB-suspend→sleep delay — is a USB-block timing detail not visible in
  the static image.)*

### RF timer — `[0x06C8:0x06C9]`, ticked by Timer2
- Incremented by the **Timer2 ISR** (`0x2C74`) — the same periodic LED/matrix-scan
  ISR, so it advances once per scan frame.
- Compared (`0x98C1`, in RF mode only) against a **configurable threshold
  `[0x0417:0x0418]`** (loaded in the `0x56xx` config region; high byte ≈ `0x07`).
  On reaching it: reset the counter and set `_4_1` (bit `0x21`), the RF sleep
  request.
- A secondary RF path sets `_2_1` (bit `0x11`) at `0x5846` from a `[0x0371]` vs
  `[0x0AFF]` comparison (RF-state/▿signal dependent).

### Activity reset
Both counters are zeroed on activity. `[0x0BEF/BF0]` resets at `0x0034`, `0x0184`
(which also clears the `0x5A` request), `0x6100`, and at sleep entry. `[0x06C8/C9]`
resets at `0x60ED` and on wake (`0x7BC9`). Net effect: a keypress restarts the
idle timer.

### The decision (main loop `0x779A`)
```c
// USB sleep:
if ( (_b_2 /*0x5A*/ || timer[0x0373:0x0374] >= ~209) && [0x039F]==0 /*USB*/ )
    stock_sleep_inactivity_entry();   // 0x8DFD — USB variant

// RF sleep:
if ( (_4_1 /*0x21*/ && [0x039F]!=0 /*RF*/) || _2_1 /*0x11*/ )
    stock_sleep_rf_entry();           // 0x7A7D — RF variant
```
`[0x0373:0x0374]` is a second USB timer (reset on SOF in the USB ISR, threshold
~`0xD1`); `[0x0366:0x0367]` (threshold ~`0xC8`) paces the RF status poll, not sleep.

### One-line summary
> **USB mode:** sleep when the host suspends the bus (SUSPIF-driven counter) —
> never while it's actively polling. **RF mode:** sleep after an inactivity
> timeout (Timer2-ticked counter vs a configurable threshold), reset by any key.
> Wake on a keypress (INT4) — or a USB resume in USB mode — and signal USB
> remote-wakeup on the way back.

## 4. How SMK maps onto this

SMK's feature lives in `src/smk/sleep.c`, `src/platform/sh68f90a/power.c`, and the
board hook `src/keyboards/<kb>/user_sleep.c` (gated by the meson `sleep` feature).

| stock | SMK | match |
|---|---|---|
| two PD variants by slider | `power_enter_powerdown(usb_keep_alive)`, `user_sleep_supported()`→`USER_SLEEP_RF`/`USB` | ✅ faithful |
| RF: regulator off + `usb_init` re-enumerate on wake | same | ✅ |
| USB: GOSUSP + resume (no re-enumerate) | same | ✅ |
| GPIO park + INT4 arm | `user_sleep_prepare()` (nuphy: verbatim transcription of `0x8E1A…`) | ✅ |
| wake clock = warm-up *then* select PLL | `clock_init()` — polls `PLLSTA` instead of stock's fixed NOP spin | ≈ equivalent |
| MOT-pin (P0.5) restore on wake | `user_sleep_wake()` | ✅ |
| RF timer = Timer2-ISR counter vs threshold, reset on key | `sleep_tick()` (Timer2 ISR) + `SLEEP_TIMEOUT` + `sleep_note_activity()` | ≈ same mechanism; **stock's threshold is configurable (`[0x0417]`), SMK's is a fixed `#define`** |
| USB timer = SUSPIF-driven | `usb_suspended` (set on `SUSPIF` when configured, cleared on SOF) gates USB sleep | ≈ same intent (flag vs counter) |
| INT4 ISR sets `_c_3`→`WKUP` | `int4_isr` sets `int4_woke`; wake raises `WKUP` iff an INT4 keypress woke us | ✅ aligned (stock's `_c_3` behaviour — always remote-wakes on a keypress wake) |
| BK3632 sleep-notify + wake re-sync loop | `rf_wake_from_sleep()` before; **inline 10× nudge+status loop** after (`resync_tries` in `__xdata`/XRAM to stay off the IRAM-overlaid stack) | ✅ aligned |

### Remaining differences (deliberate — utility matched, not bit-exact)
- **RF timeout is a fixed `SLEEP_TIMEOUT`**, not stock's configurable threshold
  `[0x0417/0418]`. Same utility (sleep after idle); only the value/source differ.
- **Secondary timer paths not replicated**: `[0x0373/0374]` (2nd USB timer) and
  `_2_1`/`[0x0371]`/`[0x0AFF]` (2nd RF path) ride on stock-internal RF/USB state
  SMK doesn't keep. Their utility (sleep on USB-idle / RF-idle) is already covered
  by the primary triggers, so they're intentionally not bit-replicated.
- **Wake clock**: SMK polls `PLLSTA`; stock burns a fixed NOP warm-up. *Identical
  outcome* (PLL locked before `PLLFS`/`FS`) — the poll is just the robust form.

## 5. Stock function map (for re-tracing)
```
0x0000  reset → LJMP 0xF000 (init)
0x01FA  INT4 ISR (wake): clear flag, set _c_3
0x27BD  Timer2 ISR (LED/scan): ticks RF inactivity counter [0x06C8/C9] @0x2C74
0x779A  main-loop sleep consumer (decision)
0x7A7D  RF sleep entry  → 0x7B48 (regulator-off PD + wake + BK3632 re-sync loop)
0x8DFD  USB sleep entry (GOSUSP PD + resume)
0x96CE  slider debounce → [0x039F] mode flag
0x9C1E  USB ISR body (SUSPIF branch tail-LJMPs 0xB2F6 @0x9C6A)
0xADC4  wake warm-up (HFON+PLLON + NOP spin)
0xB2F6  USB-suspend counter tick + threshold(200)→ bit 0x5A (_b_2)
0xB308  raw 16-bit increment of [0x0BEF:0x0BF0]
0x001E  USB re-init on wake (USBADDR=0, USBIE1=0x5F, USBIE2=0x77, USBCON=0xC4)
0x98C1  RF inactivity check: [0x06C8/C9] vs [0x0417/18] → bit 0x21 (_4_1)
0x5846  RF secondary trigger → bit 0x11 (_2_1)
```
State bits: `0x5A`=`_b_2` USB-sleep request · `0x21`=`_4_1` RF-sleep request ·
`0x11`=`_2_1` RF secondary · `0x63`=`_c_3` remote-wakeup request · `0x33`=`_6_3`
(set unconditionally each loop — *not* a host-suspend gate, despite an earlier guess).
