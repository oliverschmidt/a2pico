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

#define SM_SYNC 3

static struct {
    uint          offset;
    pio_sm_config config;
} a2_sm[3];

static void(*a2_resethandler)(bool);

static void(*a2_synchandler)(void);

static void __time_critical_func(a2_reset)(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        pio_set_sm_mask_enabled(pio0, 7ul, false);
        pio_sm_set_pins_with_mask(pio0, 0, 3ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);

        void(*handler)(bool) = a2_resethandler;
        if (handler) {
            handler(true);
        }
    }
    else  // do not come out of reset on spikes
    if (events & GPIO_IRQ_EDGE_RISE) {
        for (uint sm = 0; sm < 3; sm++) {
            pio_sm_init(pio0, sm, a2_sm[sm].offset, &a2_sm[sm].config);
        }
        pio_set_sm_mask_enabled(pio0, 7ul, true);

        void(*handler)(bool) = a2_resethandler;
        if (handler) {
            handler(false);
        }
    }
}

void a2pico_init(void) {
    pio_gpio_init(pio0, GPIO_ENBL);
    gpio_disable_pulls(GPIO_ENBL);

    pio_gpio_init(pio0, GPIO_PHI1);
    gpio_disable_pulls(GPIO_PHI1);

    for (uint gpio = GPIO_ADDR; gpio < GPIO_ADDR + SIZE_ADDR; gpio++) {
        pio_gpio_init(pio0, gpio);
        gpio_disable_pulls(gpio);
        gpio_set_input_hysteresis_enabled(gpio, false);
    }

    pio_gpio_init(pio0, GPIO_ADDR_OE);
    pio_gpio_init(pio0, GPIO_DATA_OE);
    pio_gpio_init(pio0, GPIO_DATA_DIR);
    pio_sm_set_pindirs_with_mask(pio0, 0, 7ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);
    pio_sm_set_pins_with_mask(   pio0, 0, 3ul << GPIO_ADDR_OE, 7ul << GPIO_ADDR_OE);

    gpio_init(GPIO_RESET);
    gpio_disable_pulls(GPIO_RESET);

    gpio_init(GPIO_IRQ);
    gpio_put(GPIO_IRQ, false);  // active high
    gpio_set_dir(GPIO_IRQ, GPIO_OUT);

    a2_sm[SM_ADDR].offset = pio_add_program(pio0, &addr_program);
    a2_sm[SM_ADDR].config = addr_program_get_default_config(a2_sm[SM_ADDR].offset);
    addr_program_set_config(&a2_sm[SM_ADDR].config);

    a2_sm[SM_READ].offset = pio_add_program(pio0, &read_program);
    a2_sm[SM_READ].config = read_program_get_default_config(a2_sm[SM_READ].offset);
    read_program_set_config(&a2_sm[SM_READ].config);

    a2_sm[SM_WRITE].offset = pio_add_program(pio0, &write_program);
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

static void a2_sync(void) {
    a2_synchandler();
    pio_interrupt_clear(pio0, 0);
}

void a2pico_synchandler(void(*handler)(void), uint32_t counter) {
    static int           offset = -1;
    static pio_sm_config config;

    if (offset < 0) {
        offset = pio_add_program(pio0, &sync_program);
        config = sync_program_get_default_config(offset);
        pio_set_irq0_source_enabled(pio0, pis_interrupt0, true);
        irq_set_exclusive_handler(PIO0_IRQ_0, a2_sync);
    }

    pio_sm_set_enabled(pio0, SM_SYNC, false);
    irq_set_enabled(PIO0_IRQ_0, false);
    pio_interrupt_clear(pio0, 0);

    a2_synchandler = handler;

    if (handler != NULL) {
        pio_sm_init(pio0, SM_SYNC, offset, &config);
        pio_sm_put(pio0, SM_SYNC, counter);

        irq_set_enabled(PIO0_IRQ_0, true);
        pio_sm_set_enabled(pio0, SM_SYNC, true);
    }
}
