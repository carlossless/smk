#include "peripherals.h"
#include "sh68f881.h"

// a pin mux'ed to an analog block ignores its port registers: OPCON.OPOS takes rows P2.1-P2.3, ADCH and P5SS the anodes.
void peripherals_init(void)
{
    sfr_page_0();

    OPCON   = 0;
    ADCON   = 0;
    ADCDS   = 0;
    ADCH    = 0;
    LCDCON  = 0;
    LCDCON1 = 0;
    P5SS    = 0;
    P6SS    = 0;
    P7SS    = 0;
    P8SS    = 0;
    PXSS    = 0;
    SPCON   = 0;
    SPSTA   = 0;
    SPDAT   = 0;
}
