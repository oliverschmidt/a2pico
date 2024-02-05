/*

MIT License

Copyright (c) 2022 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <a2pico.h>

#include "board.h"

extern const __attribute__((aligned(4))) uint8_t firmware[];

volatile bool active;
volatile bool reset;

static void __time_critical_func(handler)(bool asserted) {
    if (asserted) {
        a2pico_irq(false);
        active = false;
    } else {
        reset = true;
    }
}

void __time_critical_func(board)(void) {

    a2pico_init(pio0);

    a2pico_resethandler(&handler);

    while (true) {
        uint32_t pico = a2pico_getaddr(pio0);
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;  // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;  // IOSTRB
        uint32_t read = pico & 0x1000;  // R/W

        if (read) {
            if (!io) {  // DEVSEL
                switch (addr & 0xF) {
                    case 0x0:
                        a2pico_putdata(pio0, sio_hw->fifo_rd);
                        break;
                    case 0x1:
                        // SIO_FIFO_ST_VLD_BITS _u(0x00000001)
                        // SIO_FIFO_ST_RDY_BITS _u(0x00000002)
                        a2pico_putdata(pio0, sio_hw->fifo_st << 6);
                        break;
                }
            } else {
                if (!strb || (active && (addr != 0x0FFF))) {
                    a2pico_putdata(pio0, firmware[addr]);
                }
            }
        } else {
            uint32_t data = a2pico_getdata(pio0);
            if (!io) {  // DEVSEL
                switch (addr & 0xF) {
                    case 0x0:
                        sio_hw->fifo_wr = data;
                        break;
                    case 0x1:
                        a2pico_irq(false);
                        break;
                }
            }
        }

        if (io && !strb) {
            active = true;
        } else if (addr == 0x0FFF) {
            active = false;
        }
    }
}
