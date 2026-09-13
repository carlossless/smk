// Hardware SPI master. The pins are the part's, not a choice: see its spi_hw.h. The clock
// divider and frame format come from kbdef.h, which is where a board would put them; the one
// beside this file stands in for that.

#include "clock.h"
#include "delay.h"
#include "ldo.h"
#include "peripherals.h"
#include "spi.h"
#include "watchdog.h"

void main(void)
{
    ldo_init();
    clock_init();
    peripherals_init();
    spi_init();

    uint8_t frame[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t reply[4];

    for (;;) {
        watchdog_kick();

        // a command byte, its answer, then a block each way
        uint8_t status = spi_xfer_byte(0x9F);

        spi_send(frame, sizeof(frame));
        spi_recv(reply, sizeof(reply), 0xFF);

        // full duplex: what the slave shifted back replaces what went out
        spi_xfer(frame, sizeof(frame));

        if (status == 0xFF) {
            spi_deinit();
            spi_init();
        }

        delay_ms(10);
    }
}
