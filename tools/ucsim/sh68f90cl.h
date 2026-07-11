/*
 * uCsim CPU variant: SinoWealth SH68F90 (8051 core + USB SIE, key matrix,
 * BK3632 wireless SPI, flash ISP, sleep/wake, watchdog).
 *
 * Derives from cl_uc52 (8052 core: 256 B IRAM, Timer2). Selected with -t sh68f90.
 * The peripherals live in sh68f90.cc as cl_hw subclasses; this is just the CPU
 * class the factory in sim51.cc constructs.
 */
#ifndef SH68F90CL_HEADER
#    define SH68F90CL_HEADER

#    include "ddconfig.h"
#    include "uc52cl.h"

class cl_sh68f90 : public cl_uc52
{
   public:
    cl_sh68f90(struct cpu_entry *Itype, class cl_sim *asim);
    virtual int  init(void);
    virtual void mk_hw_elements(void);
};

#endif

/* End of s51.src/sh68f90cl.h */
