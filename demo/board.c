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

#include "bus.pio.h"

extern const __attribute__((aligned(4))) uint8_t firmware[];

static bool __not_in_flash("active") active;

void __not_in_flash_func(board)() {
    while (true) {
        uint32_t enbl = pio_sm_get_blocking(pio0, sm_enbl);
        uint32_t addr = enbl & 0x0FFF;
        uint32_t io   = enbl & 0x0F00;  // IOSTRB or IOSEL
        uint32_t strb = enbl & 0x0800;  // IOSTRB
        uint32_t read = enbl & 0x1000;  // R/W

        if (addr == 0x0FFF) {
            active = false;
        }

        if (read) {
            if (!io) {  // DEVSEL
                switch (addr & 0xF) {
                    case 0x0:
                        pio_sm_put(pio0, sm_read, sio_hw->fifo_rd);
                        break;
                    case 0x1:
                        // SIO_FIFO_ST_VLD_BITS _u(0x00000001)
                        // SIO_FIFO_ST_RDY_BITS _u(0x00000002)
                        pio_sm_put(pio0, sm_read, sio_hw->fifo_st << 6);
                        break;
                }
            } else {
                if (!strb || active) {
                    pio_sm_put(pio0, sm_read, firmware[addr]);
                }
            }
        } else {
            uint32_t data = pio_sm_get_blocking(pio0, sm_write);
            if (!io) {  // DEVSEL
                switch (addr & 0xF) {
                    case 0x0:
                        sio_hw->fifo_wr = data;
                        break;
                }
            }
        }

        if (io && !strb) {
            active = true;
        }

        gpio_put(gpio_led, active);
    }
}
