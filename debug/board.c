/*

MIT License

Copyright (c) 2026 Oliver Schmidt (https://a2retro.de/)

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

void __time_critical_func(board)(void) {

    a2pico_init();

    while (true) {
        uint32_t pico = a2pico_getaddr();
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;  // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;  // IOSTRB
        uint32_t read = pico & RW_BIT;  // R/W

        uint32_t value = 0xC000 | addr;
        if (read) {
            if (!strb) {
                uint32_t data;
                if (!io) {
                    data = addr & 0x000F;
                }
                else {
                    data = addr & 0x00FF;
                }
                a2pico_putdata(data);
                value |= 0b10 << 16;
                value |= data << 18;
            }
        }
        else {
            value |= 0b01 << 16;
            value |= a2pico_getdata() << 18;
        }
        sio_hw->fifo_wr = value;
    }
}
