/*  _____          _                      ___  ____
 * |  ___|__  _ __| |_ _   _ _ __   __ _ / _ \/ ___|
 * | |_ / _ \| '__| __| | | | '_ \ / _` | | | \___ \
 * |  _| (_) | |  | |_| |_| | | | | (_| | |_| |___) |
 * |_|  \___/|_|   \__|\__,_|_| |_|\__,_|\___/|____/
 *
 */


#include "os.h"
#include "debug.h"
#include "rios.h"
#include "ruota.h"

void os_init(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    DDRB  |=  _BV(PB7);  	 /* LED as output */

    init_debug_uart1();
    init_lcd();
    os_init_scheduler();
    os_init_ruota();
}
