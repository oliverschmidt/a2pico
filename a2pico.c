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

#include "a2pico.pio.h"

#include "a2pico.h"

void a2pico_init(PIO pio) {

    gpio_init(GPIO_ENBL);
    gpio_set_pulls(GPIO_ENBL, false, false);  // floating

    for (uint gpio = GPIO_ADDR; gpio < GPIO_ADDR + SIZE_ADDR; gpio++) {
        gpio_init(gpio);
        gpio_set_pulls(gpio, false, false);  // floating
    }

    for (uint gpio = GPIO_DATA; gpio < GPIO_DATA + SIZE_DATA; gpio++) {
        pio_gpio_init(pio, gpio);
        gpio_set_pulls(gpio, false, false);  // floating
    }

    gpio_init(GPIO_IRQ);
    gpio_pull_up(GPIO_IRQ);

    gpio_init(GPIO_RES);
    gpio_pull_up(GPIO_RES);

    uint offset;

    offset = pio_add_program(pio0, &addr_program);
    addr_program_init(pio, offset);

    offset = pio_add_program(pio0, &write_program);
    write_program_init(pio, offset);

    offset = pio_add_program(pio0, &read_program);
    read_program_init(pio, offset);
}
