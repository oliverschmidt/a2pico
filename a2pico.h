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

#ifndef _A2PICO_H
#define _A2PICO_H

#include <hardware/pio.h>

#define SM_ADDR  0
#define SM_READ  1
#define SM_WRITE 2

#define GPIO_PHI1  16
#define GPIO_RESET 17
#define GPIO_IRQ   18

#define GPIO_SPI0_TX  19
#define GPIO_SPI0_RX  20
#define GPIO_SPI0_CSN 21
#define GPIO_SPI0_SCK 22

void a2pico_init(PIO pio);

void a2pico_resethandler(void(*handler)(bool asserted));

static __always_inline uint32_t a2pico_getaddr(PIO pio) {
    while (pio->fstat & (1u << (PIO_FSTAT_RXEMPTY_LSB + SM_ADDR))) {
        tight_loop_contents();
    }
    return pio->rxf[SM_ADDR];
}

static __always_inline uint32_t a2pico_getdata(PIO pio) {
    uint retry = 32;
    while (pio->fstat & (1u << (PIO_FSTAT_RXEMPTY_LSB + SM_WRITE)) && --retry) {
        tight_loop_contents();
    }
    return pio->rxf[SM_WRITE];
}

static __always_inline void a2pico_putdata(PIO pio, uint32_t data) {
    pio->txf[SM_READ] = data;
}

static __always_inline void a2pico_irq(bool assert) {
    gpio_put(GPIO_IRQ, assert);  // active high
}

#endif
