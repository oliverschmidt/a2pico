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

static PIO a2_pio;

static struct {
    uint          offset;
    pio_sm_config config;
} a2_sm[3];

static void(*a2_resethandler)(bool);

static void __time_critical_func(a2_reset)(uint gpio, uint32_t events) {

    if (events & GPIO_IRQ_EDGE_FALL) {
        pio_set_sm_mask_enabled(a2_pio, 7ul, false);
        pio_sm_set_pins_with_mask(a2_pio, SM_ADDR, 3ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);

        void(*handler)(bool) = a2_resethandler;
        if (handler) {
            handler(true);
        }
    }
    else  // do not come out of reset on spikes
    if (events & GPIO_IRQ_EDGE_RISE) {
        for (uint sm = 0; sm < 3; sm++) {
            pio_sm_init(a2_pio, sm, a2_sm[sm].offset, &a2_sm[sm].config);
        }
        pio_set_sm_mask_enabled(a2_pio, 7ul, true);

        void(*handler)(bool) = a2_resethandler;
        if (handler) {
            handler(false);
        }
    }
}

void a2pico_init(PIO pio) {
    a2_pio = pio;

    pio_gpio_init(pio, GPIO_ENBL);
    gpio_disable_pulls(GPIO_ENBL);

    for (uint gpio = GPIO_ADDR; gpio < GPIO_ADDR + SIZE_ADDR; gpio++) {
        pio_gpio_init(pio, gpio);
        gpio_disable_pulls(gpio);
        gpio_set_input_hysteresis_enabled(gpio, false);
    }

    pio_gpio_init(pio, GPIO_ADDR_OE);
    pio_gpio_init(pio, GPIO_DATA_OE);
    pio_gpio_init(pio, GPIO_DATA_DIR);
    pio_sm_set_pindirs_with_mask(pio, SM_ADDR, 7ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);
    pio_sm_set_pins_with_mask(   pio, SM_ADDR, 3ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);

    gpio_init(GPIO_PHI1);
    gpio_disable_pulls(GPIO_PHI1);

    gpio_init(GPIO_RESET);
    gpio_disable_pulls(GPIO_RESET);

    gpio_init(GPIO_IRQ);
    gpio_put(GPIO_IRQ, false);  // active high
    gpio_set_dir(GPIO_IRQ, GPIO_OUT);

    a2_sm[SM_ADDR].offset = pio_add_program(pio, &addr_program);
    a2_sm[SM_ADDR].config = addr_program_get_default_config(a2_sm[SM_ADDR].offset);
    addr_program_set_config(&a2_sm[SM_ADDR].config);

    a2_sm[SM_READ].offset = pio_add_program(pio, &read_program);
    a2_sm[SM_READ].config = read_program_get_default_config(a2_sm[SM_READ].offset);
    read_program_set_config(&a2_sm[SM_READ].config);

    a2_sm[SM_WRITE].offset = pio_add_program(pio, &write_program);
    a2_sm[SM_WRITE].config = write_program_get_default_config(a2_sm[SM_WRITE].offset);
    write_program_set_config(&a2_sm[SM_WRITE].config);

    gpio_set_irq_enabled_with_callback(GPIO_RESET, GPIO_IRQ_EDGE_FALL
                                                 | GPIO_IRQ_EDGE_RISE, true, a2_reset);
    if (gpio_get(GPIO_RESET)) {
        a2_reset(GPIO_RESET, GPIO_IRQ_EDGE_RISE);
    }                                                 
}

void a2pico_resethandler(void(*handler)(bool)) {
    a2_resethandler = handler;
}
