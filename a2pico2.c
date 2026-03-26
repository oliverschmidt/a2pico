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

#include "a2pico2.pio.h"

#include "a2pico.h"

// PIO0
#define SM_DEVSEL 0
#define SM_IOSEL  1
#define SM_IOSTRB 2

// PIO1
#define SM_SYNC   2

static void(*a2_synchandler)(void);

void a2pico_init(void) {
    for (uint gpio = GPIO_DEVSEL; gpio < GPIO_DEVSEL + SIZE_ENBL; gpio++) {
        pio_gpio_init(pio0, gpio);
        gpio_disable_pulls(gpio);
        gpio_set_input_hysteresis_enabled(gpio, true);
    }

    for (uint gpio = GPIO_ADDR; gpio < GPIO_ADDR + SIZE_ADDR; gpio++) {
        pio_gpio_init(pio1, gpio);
        gpio_disable_pulls(gpio);
        gpio_set_input_hysteresis_enabled(gpio, false);
        gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);
    }
    
    pio_gpio_init(pio1, GPIO_PHI0);
    gpio_disable_pulls(GPIO_PHI0);

    gpio_init(GPIO_IRQ);
    gpio_disable_pulls(GPIO_IRQ);
    gpio_put(GPIO_IRQ, true);  // active low
    gpio_set_dir(GPIO_IRQ, GPIO_OUT);

    uint          offset;
    pio_sm_config config;

    offset = pio_add_program(pio0, &devsel_program);
    config = devsel_program_get_default_config(offset);
    pio_sm_init(pio0, SM_DEVSEL, offset, &config);

    offset = pio_add_program(pio0, &iosel_program);
    config = iosel_program_get_default_config(offset);
    pio_sm_init(pio0, SM_IOSEL, offset, &config);

    offset = pio_add_program(pio0, &iostrb_program);
    config = iostrb_program_get_default_config(offset);
    pio_sm_init(pio0, SM_IOSTRB, offset, &config);

    offset = pio_add_program(pio0, &addr_program);
    config = addr_program_get_default_config(offset);
    addr_program_set_config(&config);
    pio_sm_init(pio0, SM_ADDR, offset, &config);

    offset = pio_add_program(pio1, &read_program);
    config = read_program_get_default_config(offset);
    read_program_set_config(&config);
    pio_sm_init(pio1, SM_READ, offset, &config);

    offset = pio_add_program(pio1, &write_program);
    config = write_program_get_default_config(offset);
    write_program_set_config(&config);
    pio_sm_init(pio1, SM_WRITE, offset, &config);

    pio_set_sm_mask_enabled(pio0, 0b1111, true);
    pio_set_sm_mask_enabled(pio1, 0b0011, true);
}

void a2pico_resethandler(void(*handler)(bool)) {
}

static void a2_sync(void) {
    a2_synchandler();
    pio_interrupt_clear(pio1, 0);
}

void a2pico_synchandler(void(*handler)(void), uint32_t counter) {
    static int           offset = -1;
    static pio_sm_config config;

    if (offset < 0) {
        offset = pio_add_program(pio1, &sync_program);
        config = sync_program_get_default_config(offset);
        pio_set_irq0_source_enabled(pio1, pis_interrupt0, true);
        irq_set_exclusive_handler(PIO1_IRQ_0, a2_sync);
    }

    pio_sm_set_enabled(pio1, SM_SYNC, false);
    irq_set_enabled(PIO1_IRQ_0, false);
    pio_interrupt_clear(pio1, 0);

    a2_synchandler = handler;

    if (handler != NULL) { 
        pio_sm_init(pio1, SM_SYNC, offset, &config);
        pio_sm_put(pio1, SM_SYNC, counter);

        irq_set_enabled(PIO1_IRQ_0, true);
        pio_sm_set_enabled(pio1, SM_SYNC, true);
    }
}
