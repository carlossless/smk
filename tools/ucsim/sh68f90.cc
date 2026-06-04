/*
 * uCsim CPU variant: SinoWealth SH68F90  (-t sh68f90)
 *
 * An 8052 core (cl_uc52) plus a single cl_hw peripheral model (cl_sh68f90_sie)
 * that emulates the chip-specific blocks the NuPhy Air60 stock firmware drives:
 *   - USB SIE      : EP0 control/enumeration, EP1/EP2 IN report endpoints
 *   - Key matrix   : pin-level (P7/P5 rows, P5/P3/P2/P1 cols), NKRO
 *   - BK3632 SPI   : bit-bang ACK (P4.2) + MISO status + connection state
 *   - Flash ISP    : IB_CON/XPAGE/IB_OFFSET/IB_DATA erase+program into code space
 *   - Sleep/wake   : PCON power-down + INT4 (EXF1) wake
 *   - Watchdog     : RSTSTAT kick + timeout reset
 * plus a cl_sh68f90_interrupt that registers the SH68F90's (remapped) interrupt
 * vectors instead of the standard 8051 INT0/INT1.
 *
 * This replaces the old approach of patching the shared s51 core files
 * (interrupt.cc / uc51.cc): everything chip-specific now lives here, and the CPU
 * is registered as a normal uCsim variant (cpus_51[] + sim51.cc factory).
 */
#include <stdio.h>

#include "globals.h"
#include "regs51.h"
#include "dregcl.h"
#include "portcl.h"
#include "timer2cl.h"
#include "interruptcl.h"
#include "itsrccl.h"

#include "uc52cl.h"
#include "sh68f90cl.h"


/* ===================================================================== *
 *  SH68F90 peripheral model (USB SIE + matrix + BK3632 + flash + power)  *
 * ===================================================================== */
class cl_sh68f90_sie: public cl_hw
{
  class cl_address_space *xram, *sfr, *iram, *rom;
  class cl_memory_cell *cell_ep0con, *cell_usbif2, *cell_iep0cnt;
  class cl_memory_cell *cell_ep1con, *cell_iep1cnt;
  class cl_memory_cell *cell_ep2con, *cell_iep2cnt;
  class cl_memory_cell *cell_pllcon;
  class cl_memory_cell *cell_sbuf, *cell_scon;
  class cl_memory_cell *cell_p1, *cell_p2, *cell_p3, *cell_p5, *cell_p7;
  class cl_memory_cell *cell_p0, *cell_p4;
  class cl_memory_cell *cell_ibcon5, *cell_rststat, *cell_pcon;
  unsigned pwm_acc;
  int in_packets;
  bool rf_ack_toggle;       // BK3632 SPI: P4.2 ACK flips each pin-read so the
                            // firmware's "wait for ACK to change" poll matches.
  int  miso_bitpos;         // BK3632 SPI: bit cursor into the 4-byte status reply.
  unsigned wdt_acc;         // watchdog: cycles since last RSTSTAT(0xb1) kick.
  bool wdt_armed;           // watchdog only enforced after the firmware kicks once.

  // External level present on each port's pins (what the board drives). An input
  // pin reads this, not its output latch; it idles high (== the internal pull-up
  // a real input enables via PxPCR). The board model (tests/) pulls bits low via
  // the staging cells registered in init(); nothing board-specific lives here.
  t_mem pin_ext[8];
  class cl_memory_cell *cell_pinext_p5, *cell_pinext_p7;
  // Bit cells for the bit-addressable input pins of P5/P7. Bit reads bypass the
  // byte read() operator, so we hook these too (as cl_port does) -- otherwise a
  // `MOV C,P5.5` (CONN_MODE switch) reads the latch instead of the pin level.
  class cl_memory_cell *p5_bit[8], *p7_bit[8];

public:
  cl_sh68f90_sie(class cl_uc *auc):
    cl_hw(auc, HW_DUMMY, 0, "sh68f90_sie")
  {
    xram= sfr= iram= rom= 0;
    cell_ibcon5= cell_rststat= cell_pcon= 0;
    wdt_acc= 0;
    wdt_armed= false;
    cell_ep0con= cell_usbif2= cell_iep0cnt= 0;
    cell_ep1con= cell_iep1cnt= 0;
    cell_ep2con= cell_iep2cnt= 0;
    cell_pllcon= 0;
    cell_sbuf= cell_scon= 0;
    cell_p1= cell_p2= cell_p3= cell_p5= cell_p7= 0;
    cell_p0= cell_p4= 0;
    pwm_acc= 0;
    in_packets= 0;
    rf_ack_toggle= false;
    miso_bitpos= 0;
    for (int i= 0; i < 8; i++) pin_ext[i]= 0xff;   // pins idle high (pull-ups)
    cell_pinext_p5= cell_pinext_p7= 0;
    for (int i= 0; i < 8; i++) p5_bit[i]= p7_bit[i]= 0;
  }
  virtual int init(void)
  {
    cl_hw::init();
    xram= uc->address_space("xram");
    sfr = uc->address_space(MEM_SFR_ID);
    iram= uc->address_space("iram");
    rom = uc->address_space("rom");          // code/flash space (for ISP erase/program)
    if (sfr)
      {
	cell_ep0con  = register_cell(sfr, 0x97); // EP0CON
	cell_usbif2  = sfr->get_cell(0x93);      // USBIF2
	cell_iep0cnt = sfr->get_cell(0x9b);      // IEP0CNT (IN byte count)
	cell_ep1con  = register_cell(sfr, 0x99); // EP1CON (keyboard report endpoint)
	cell_iep1cnt = sfr->get_cell(0x9c);      // IEP1CNT
	cell_ep2con  = register_cell(sfr, 0x9a); // EP2CON (IF1 multiplexed IN: NKRO etc.)
	cell_iep2cnt = sfr->get_cell(0x9d);      // IEP2CNT
	cell_pllcon  = register_cell(sfr, 0xbc); // PLLCON (clock PLL)
	cell_sbuf    = register_cell(sfr, 0xaa); // SBUF (real UART TX data)
	cell_scon    = sfr->get_cell(0xd8);      // SCON (TI = bit1)
	// GPIO port cells (plain MCU ports). What's wired to them -- the key
	// matrix columns/rows, the BK3632 -- is board-level and lives test-side.
	cell_p1      = sfr->get_cell(0x90);
	cell_p2      = sfr->get_cell(0x98);
	cell_p3      = sfr->get_cell(0xa0);
	cell_p5      = register_cell(sfr, 0x88); // P5: cols C0-2 + rows R3-R4
	cell_p7      = register_cell(sfr, 0xf8); // P7: rows R0-R2 (bits 1-3)
	cell_p0      = register_cell(sfr, 0x80); // P0: BK3632 MISO=P0.6, MOSI=P0.7, MOT=P0.5
	cell_p4      = register_cell(sfr, 0xb0); // P4: BK3632 SCK=P4.7, ACK=P4.2
	cell_ibcon5  = register_cell(sfr, 0xf6); // IB_CON5: flash ISP commit (write 0x06)
	cell_rststat = register_cell(sfr, 0xb1); // RSTSTAT: watchdog kick (write 0)
	cell_pcon    = register_cell(sfr, 0x87); // PCON: bit1 -> sleep/power-down
      }
    if (xram)
      {
	// Staging for the external pin levels of P5 / P7 (the ports the firmware
	// reads as inputs). The board model writes these; read() applies them to
	// the input bits. Outside the firmware's xram window, so they never alias
	// real data.
	cell_pinext_p5= register_cell(xram, 0x1f15);
	cell_pinext_p7= register_cell(xram, 0x1f17);
      }
    class cl_address_space *bas= uc->address_space("bits");
    if (bas)
      {
	// The firmware reads some pins bit-wise (P5: rows R3/R4 b3/b4, CONN_MODE
	// b5, OS switch b6; P7: rows R0-R2 b1-b3). Hook those bit cells so bit
	// reads see the pin level too. (Bit addr of Px.i = Px + i; these are all
	// inputs, so no bit-write linkage to maintain.)
	int p5in[]= {3, 4, 5, 6}, p7in[]= {1, 2, 3};
	for (int k= 0; k < 4; k++) p5_bit[p5in[k]]= register_cell(bas, 0x88 + p5in[k]);
	for (int k= 0; k < 3; k++) p7_bit[p7in[k]]= register_cell(bas, 0xf8 + p7in[k]);
      }
    return 0;
  }

  // A port read must honour pin direction: PxCR selects output(1)/input(0); an
  // output bit reads its latch, an input bit reads the external pin level (which
  // idles high via the pull-up, modelled as pin_ext defaulting to 0xff and pulled
  // low by the board test-side). This models only the MCU's own I/O behaviour --
  // what is wired to the pins (key matrix, CONN_MODE switch, BK3632) is test-side.
  t_mem port_read(class cl_memory_cell *cell, int n, t_addr cr_addr)
  {
    t_mem latch= cell->get();
    t_mem cr= sfr ? sfr->get(cr_addr) : 0;          // PxCR: 1=output, 0=input
    return (latch & cr) | (pin_ext[n] & (t_mem)(~cr & 0xff));
  }
  virtual t_mem read(class cl_memory_cell *cell)
  {
    if (cell == cell_p5) return port_read(cell, 5, 0xe6);   // P5CR @ 0xe6
    if (cell == cell_p7) return port_read(cell, 7, 0xd1);   // P7CR @ 0xd1
    for (int i= 0; i < 8; i++)
      {
	if (p5_bit[i] && cell == p5_bit[i]) return (port_read(cell_p5, 5, 0xe6) >> i) & 1;
	if (p7_bit[i] && cell == p7_bit[i]) return (port_read(cell_p7, 7, 0xd1) >> i) & 1;
      }
    return cl_hw::read(cell);
  }

  // What is wired to the pins (key matrix, BK3632, USB host, CONN_MODE switch)
  // is NOT modelled here -- the board model lives test-side (tests/devices.py:
  // KeyMatrix), driving pin_ext via the staging cells above. The chip only models
  // its own pins (direction/latch/pull-up via read()), staying board-agnostic.

  // PWM0 timebase: periodically request the PWM interrupt (vector 0x43) that
  // drives matrix_scan_step(), once the firmware has enabled it (IEN1._EPWM0).
  virtual int tick(int cycles)
  {
    if (xram && sfr)
      {
	// Watchdog: count cycles since the last RSTSTAT kick (paused during sleep,
	// when the clock stops). Only armed once the firmware has kicked it at least
	// once -- a firmware that never touches RSTSTAT (e.g. SMK) is left alone.
	if (wdt_armed && !(sfr->get(0x87) & 0x02))
	  {
	    wdt_acc += cycles;
	    if (wdt_acc > 80000000u)
	      { fprintf(stderr, "[SIE] WATCHDOG timeout -> reset\n"); wdt_acc= 0; uc->reset(); }
	  }
	// Timer ticks. These are the chip's own timers, just calibrated for uCsim:
	// raise PWM0 (vector 0x43, SMK scan) and the Timer2 1 ms overflow (vector
	// 0x0003, stock scan) at a rate the ISRs can keep up with. (External-hardware
	// effects of those scans -- the key matrix, the BK3632 -- are modelled
	// test-side; INT4 wake is likewise triggered test-side by raising EXF1.)
	pwm_acc += cycles;
	// period must exceed the matrix-scan ISR duration or the main code starves
	if (pwm_acc >= 30000)
	  {
	    pwm_acc= 0;
	    if (sfr->get(0xa9) & 0x02) // IEN1._EPWM0
	      xram->set(0x1f08, xram->get(0x1f08) | 0x01);
	    // Matrix-scan tick: the SH68F90 Timer2 ISR @0x27bd (vector 0x0003) is
	    // the 1ms scan handler. uCsim's own Timer2 overflows ~12x too fast for
	    // its modelled clock, so the scan+LED ISR storms and starves the main
	    // loop. Drive it instead from this calibrated virtual flag (0x1f09) at a
	    // rate the ISR can keep up with, gated on the scan enable IEN0(0xa8).bit0
	    // and T2CON.TR2(0x04). clr_bit=true on the it_src auto-clears the flag.
	    if ((sfr->get(0xa8) & 0x01) && (sfr->get(0xc8) & 0x04))
	      xram->set(0x1f09, xram->get(0x1f09) | 0x01);
	  }
      }
    return 0;
  }
  virtual void write(class cl_memory_cell *cell, t_mem *val)
  {
    // Board model sets the external pin levels for P5 / P7 via these staging
    // cells (read() applies them to the input bits).
    if (cell == cell_pinext_p5) pin_ext[5]= *val & 0xff;
    if (cell == cell_pinext_p7) pin_ext[7]= *val & 0xff;
    // Flash ISP: the firmware writes the IB register file then commits with
    // IB_CON5(0xf6)=0x06. Opcode in IB_CON1(0xf2): 0xe6=erase, 0x6e=program.
    // Address = XPAGE(0xf7)<<8 | IB_OFFSET(0xfb); program data = IB_DATA(0xfc).
    // Erase granularity = 512 B (XPAGE = sector*2 -> base = XPAGE*0x100).
    if (cell == cell_ibcon5 && (*val) == 0x06 && rom && sfr)
      {
	t_mem op= sfr->get(0xf2);
	unsigned base= (unsigned)sfr->get(0xf7) << 8;
	if (op == 0xe6)                       // ERASE: fill the 512 B sector with 0xff
	  for (unsigned i= 0; i < 0x200; i++) rom->set((base + i) & 0xffff, 0xff);
	else if (op == 0x6e)                  // PROGRAM: AND one byte (real flash can't set 1s)
	  {
	    unsigned a= (base | (sfr->get(0xfb) & 0xff)) & 0xffff;
	    rom->set(a, rom->get(a) & sfr->get(0xfc));
	  }
      }
    // Watchdog kick: any RSTSTAT(0xb1) write (firmware writes 0) reloads the WDT.
    if (cell == cell_rststat)
      { wdt_acc= 0; wdt_armed= true; }
    // Sleep: PCON(0x87) bit1 set = power-down/STOP (after SUSLO=0x55). The core
    // halts here until INT4 (matrix-wake) fires; the wake is injected from tick().
    if (cell == cell_pcon && ((*val) & 0x02))
      fprintf(stderr, "[SIE] sleep: PCON power-down (wake on INT4 / key)\n");
    if (cell == cell_ep0con && ((*val) & 0x04)) // IEP0RDY: firmware queued IN data
      {
	t_mem n= cell_iep0cnt ? cell_iep0cnt->get() : 0;
	fprintf(stderr, "[SIE] EP0 IN[%d] %u bytes:", in_packets, (unsigned)n);
	for (t_mem i= 0; i < n && i < 8; i++)
	  fprintf(stderr, " %02x", (unsigned)(xram->get(0x1108 + i) & 0xff));
	fprintf(stderr, "\n");
	in_packets++;
	*val &= ~0x04; // host consumed the packet -> clear ready
	if (in_packets < 16 && cell_usbif2)
	  cell_usbif2->set(cell_usbif2->get() | 0x01); // IEP0IF -> next chunk
      }
    // EP1 = the keyboard's interrupt-IN report endpoint (single-packet)
    if (cell == cell_ep1con && ((*val) & 0x04)) // IEP1RDY
      {
	t_mem n= cell_iep1cnt ? cell_iep1cnt->get() : 0;
	fprintf(stderr, "[SIE] EP1 IN %u bytes:", (unsigned)n);
	for (t_mem i= 0; i < n && i < 16; i++)
	  fprintf(stderr, " %02x", (unsigned)(xram->get(0x1120 + i) & 0xff));
	fprintf(stderr, "\n");
	*val &= ~0x04; // host consumed the report -> clear ready
      }
    // EP2 = the IF1 multiplexed interrupt-IN endpoint (NKRO keyboard + others,
    // each prefixed with a Report ID). FIFO at 0x1180, length in IEP2CNT.
    if (cell == cell_ep2con && ((*val) & 0x04)) // IEP2RDY
      {
	t_mem n= cell_iep2cnt ? cell_iep2cnt->get() : 0;
	fprintf(stderr, "[SIE] EP2 IN %u bytes:", (unsigned)n);
	for (t_mem i= 0; i < n && i < 24; i++)
	  fprintf(stderr, " %02x", (unsigned)(xram->get(0x1180 + i) & 0xff));
	fprintf(stderr, "\n");
	*val &= ~0x04; // host consumed the report -> clear ready
      }
    // Clock PLL: report "locked" (PLLSTA) the moment firmware enables it (PLLON),
    // so clock_init()'s `while (!(PLLCON & _PLLSTA))` spin returns and the real
    // boot path (init -> usb_init -> main) runs instead of deadlocking.
    if (cell == cell_pllcon && ((*val) & 0x02)) // _PLLON -> set _PLLSTA
      *val |= 0x04;
    // Real SH68F90 UART TX: a write to SBUF (0xaa) "transmits" instantly -- echo
    // the byte and raise SCON.TI (0xd8 bit1) so the UART ISR clears uart_tx_busy
    // and the firmware's blocking putchar() returns (else dprintf hangs forever).
    if (cell == cell_sbuf)
      {
	putc((char)(*val & 0xff), stderr);
	if (cell_scon)
	  cell_scon->set(cell_scon->get() | 0x02);
      }
  }
};


/* ===================================================================== *
 *  Interrupt controller: SH68F90 vectors (no standard INT0/INT1)         *
 * ===================================================================== */
class cl_sh68f90_interrupt: public cl_interrupt
{
public:
  cl_sh68f90_interrupt(class cl_uc *auc): cl_interrupt(auc) {}
  virtual int init(void);
  virtual void added_to_uc(void);
};

int
cl_sh68f90_interrupt::init(void)
{
  cl_hw::init();
  sfr= uc->address_space(MEM_SFR_ID);
  // SH68F90 has no standard INT0/INT1, and 0x88/0x8a are P5/MAPPING (owned by the
  // SIE). Register only IE; bind cell_tcon/it0/it1 via get_cell (NOT register_cell,
  // so we don't become an operator on P5). The base class derefs these, so they
  // must be non-null even though we add no INT0/INT1 sources.
  if (sfr)
    {
      register_cell(sfr, IE);
      cell_tcon= sfr->get_cell(TCON);
      bit_INT0= 0;
      bit_INT1= 0;
      cell_it0= sfr->get_cell(TCON);
      cell_it1= sfr->get_cell(TCON);
    }
  return 0;
}

void
cl_sh68f90_interrupt::added_to_uc(void)
{
  class cl_address_space *sfr= uc->address_space(MEM_SFR_ID);
  class cl_it_src *is;
  // SH68F90 USB interrupt (_INT_USB = vector 7 @ 0x003B).
  // enable: IEN1(0xa9).EUSB(bit0); request: USBIF1(0x92).SETUPIF(bit4).
  // level-triggered (clr_bit=false): the firmware ISR clears the flag.
  uc->it_sources->add(is= new cl_it_src(uc, 0x100,
					sfr->get_cell(0xa9), 0x01,
					sfr->get_cell(0x92), 0x10,
					0x003b, false, false,
					"USB (SH68F90)", 7));
  is->init();
  // EP0 IN/OUT completion sources at the same USB vector (USBIF2), so the SIE
  // model can advance control transfers and the ISP set-report status stage.
  // uCsim's pending() is (flag & mask) == mask, so each source needs a
  // single-bit mask -- a combined mask would require all those bits set at once.
  uc->it_sources->add(is= new cl_it_src(uc, 0x101,
					sfr->get_cell(0xa9), 0x01,
					sfr->get_cell(0x93), 0x01, // IEP0IF
					0x003b, false, false,
					"USB EP0-IN (SH68F90)", 7));
  is->init();
  uc->it_sources->add(is= new cl_it_src(uc, 0x102,
					sfr->get_cell(0xa9), 0x01,
					sfr->get_cell(0x93), 0x10, // OEP0IF
					0x003b, false, false,
					"USB EP0-OUT (SH68F90)", 7));
  is->init();
  // SH68F90 UART TX-complete interrupt (_INT_EUART0 = vector 13 @ 0x6B).
  // enable IEN1(0xa9)._ES0(0x40); request SCON(0xd8).TI(0x02). The SIE sets TI on
  // each SBUF write, so this fires and uart_interrupt_handler clears uart_tx_busy.
  uc->it_sources->add(is= new cl_it_src(uc, 0x103,
					sfr->get_cell(0xa9), 0x40,
					sfr->get_cell(0xd8), 0x02,
					0x006b, false, false,
					"UART TI (SH68F90)", 13));
  is->init();
  // SH68F90 PWM0 interrupt (_INT_PWM0 = vector 8 @ 0x43), which drives the matrix
  // scan. enable IEN1(0xa9)._EPWM0(0x02); request a virtual flag at xram 0x1f08
  // that the SIE's tick() raises periodically; clr_bit=true (HW auto-clears, the
  // firmware ISR doesn't).
  uc->it_sources->add(is= new cl_it_src(uc, 0x104,
					sfr->get_cell(0xa9), 0x02,
					uc->address_space("xram")->get_cell(0x1f08), 0x01,
					0x0043, true, false,
					"PWM0 (SH68F90)", 8));
  is->init();
  // SH68F90 Timer2 overflow is remapped to the INT0 vector slot (0x0003): the
  // 1 ms Timer2 ISR @0x27bd is the LED-PWM-mux + MATRIX-SCAN handler. uCsim's
  // own Timer2 runs (auto-reload) and sets T2CON.TF2(0x80) on overflow but would
  // fire the standard 0x2B vector, which this firmware doesn't use. Route TF2 ->
  // 0x0003 instead, enabled by IEN0(0xa8).bit0; clr_bit=false because the
  // firmware ISR clears TF2 itself (CLR 0xcf). This drives the key-matrix scan
  // that populates the row bitmap at EXTMEM 0x06b0.
  uc->it_sources->add(is= new cl_it_src(uc, 0x105,
					sfr->get_cell(0xa8), 0x01,
					uc->address_space("xram")->get_cell(0x1f09), 0x01,
					0x0003, true, false,
					"Timer2 scan (SH68F90)", 1));
  is->init();
  // INT4 = matrix/RF wake (vector 0x000b). Enable IEN0(0xa8).EX4(bit1), armed only
  // by the sleep path; request via EXF1(0xe8) sub-flags IF40..IF47 (raised from the
  // SIE tick() when a key is staged while asleep). ISR clears EXF1 -> clr_bit=false.
  uc->it_sources->add(is= new cl_it_src(uc, 0x106,
					sfr->get_cell(0xa8), 0x02,
					sfr->get_cell(0xe8), 0xff,
					0x000b, false, false,
					"INT4 wake (SH68F90)", 2));
  is->init();
  // INT3 (0x0013) / INT2 (0x001b): real ISR bodies exist but the firmware never
  // arms them in this build; wired so they dispatch if ever enabled. Enable
  // IEN0(0xa8).EX3(bit2)/EX2(bit3); request EXF0(0xb6).bit1/bit0; ISR clears flag.
  uc->it_sources->add(is= new cl_it_src(uc, 0x107,
					sfr->get_cell(0xa8), 0x04,
					sfr->get_cell(0xb6), 0x02,
					0x0013, false, false,
					"INT3 (SH68F90)", 3));
  is->init();
  uc->it_sources->add(is= new cl_it_src(uc, 0x108,
					sfr->get_cell(0xa8), 0x08,
					sfr->get_cell(0xb6), 0x01,
					0x001b, false, false,
					"INT2 (SH68F90)", 3));
  is->init();
}


/* ===================================================================== *
 *  cl_sh68f90 : 8052 core + the peripherals above                        *
 * ===================================================================== */
cl_sh68f90::cl_sh68f90(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc52(Itype, asim)
{
}

int
cl_sh68f90::init(void)
{
  return cl_uc52::init();
}

void
cl_sh68f90::mk_hw_elements(void)
{
  // This is cl_51core::mk_hw_elements() with timer0/timer1/serial OMITTED (the
  // SH68F90 remaps SFRs 0x88-0x99 to GPIO/clock/USB, so the standard timer/UART
  // models would corrupt those), plus Timer2 (from cl_uc52), the SIE peripheral,
  // and the SH68F90 interrupt controller.
  cl_uc::mk_hw_elements();

  class cl_hw *h;
  acc= sfr->get_cell(ACC);
  psw= sfr->get_cell(PSW);

  // Timer2 (8052) -- kept; the firmware's 1 ms matrix-scan ISR rides its overflow.
  h= new cl_timer2(this, 2, "timer2", t2_default|t2_down);
  h->init();
  add_hw(h);

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();

  class cl_port_ui *d;
  add_hw(d= new cl_port_ui(this, 0, "dport"));
  d->init();

  class cl_port *p0, *p1, *p2, *p3;
  add_hw(p0= new cl_port(this, 0)); p0->init();
  add_hw(p1= new cl_port(this, 1)); p1->init();
  add_hw(p2= new cl_port(this, 2)); p2->init();
  add_hw(p3= new cl_port(this, 3)); p3->init();

  class cl_port_data pd;
  pd.init();
  pd.cell_dir= NULL;
  pd.set_name("P0"); pd.cell_p= p0->cell_p; pd.cell_in= p0->cell_in; pd.keyset= keysets[0]; pd.basx= 1;  pd.basy= 5; d->add_port(&pd, 0);
  pd.set_name("P1"); pd.cell_p= p1->cell_p; pd.cell_in= p1->cell_in; pd.keyset= keysets[1]; pd.basx= 20; pd.basy= 5; d->add_port(&pd, 1);
  pd.set_name("P2"); pd.cell_p= p2->cell_p; pd.cell_in= p2->cell_in; pd.keyset= keysets[2]; pd.basx= 40; pd.basy= 5; d->add_port(&pd, 2);
  pd.set_name("P3"); pd.cell_p= p3->cell_p; pd.cell_in= p3->cell_in; pd.keyset= keysets[3]; pd.basx= 60; pd.basy= 5; d->add_port(&pd, 3);

  // The chip-specific peripheral model (registers its SFR cells in init()).
  cl_sh68f90_sie *sie= new cl_sh68f90_sie(this);
  add_hw(sie);
  sie->init();

  // Interrupt controller (its added_to_uc adds the SH68F90 it_sources).
  add_hw(interrupt= new cl_sh68f90_interrupt(this));
  interrupt->init();
}

/* End of s51.src/sh68f90.cc */
