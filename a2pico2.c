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

#include <hardware/adc.h>

#include "a2pico2.pio.h"

#include "a2pico.h"

#define SM_SYNC   3

// PIO1
#define SM_DEVSEL 0
#define SM_IOSEL  1
#define SM_IOSTRB 2

static volatile bool ready;

static bool wifi;

static struct {
    uint          offset;
    pio_sm_config config;
} a2_sm[3];

static void(*a2_resethandler)(bool);

static void(*a2_synchandler)(void);

static void __time_critical_func(a2_reset)(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        pio_set_sm_mask_enabled(pio0, 0b0111, false);
        // make sure to get off the bus no matter what
        pio_sm_set_pindirs_with_mask(pio0, 0, 0, 0b11111111 << GPIO_DATA);

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
        pio_set_sm_mask_enabled(pio0, 0b0111, true);

        void(*handler)(bool) = a2_resethandler;
        if (handler) {
            handler(false);
        }
    }
}

void a2pico_init(void) {
    // see 'Connecting to the Internet with Raspberry Pi Pico W-series.'
    //     section 'Which hardware am I running on?'
    adc_init();
    adc_gpio_init(29);
    adc_select_input(3);
    wifi = adc_read() < 500;
    ready = true;

    for (uint gpio = GPIO_ADDR; gpio < GPIO_ADDR + SIZE_ADDR; gpio++) {
        pio_gpio_init(pio0, gpio);
        gpio_disable_pulls(gpio);
        gpio_set_input_hysteresis_enabled(gpio, false);
        gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);
    }
    
    pio_gpio_init(pio0, GPIO_PHI0);
    gpio_disable_pulls(GPIO_PHI0);

    gpio_init(GPIO_IRQ);
    gpio_disable_pulls(GPIO_IRQ);
    gpio_set_drive_strength(GPIO_IRQ, GPIO_DRIVE_STRENGTH_12MA);
    gpio_put(GPIO_IRQ, false);  // active low

    pio_claim_sm_mask(pio0, 0b1111);  // incl. sync

    a2_sm[SM_ADDR].offset = pio_add_program(pio0, wifi ? &addr_w_program : &addr_program);
    a2_sm[SM_ADDR].config = addr_program_get_default_config(a2_sm[SM_ADDR].offset);
    addr_program_set_config(&a2_sm[SM_ADDR].config);

    a2_sm[SM_READ].offset = pio_add_program(pio0, wifi ? &read_w_program : &read_program);
    a2_sm[SM_READ].config = read_program_get_default_config(a2_sm[SM_READ].offset);
    read_program_set_config(&a2_sm[SM_READ].config);

    a2_sm[SM_WRITE].offset = pio_add_program(pio0, &write_program);
    a2_sm[SM_WRITE].config = write_program_get_default_config(a2_sm[SM_WRITE].offset);
    write_program_set_config(&a2_sm[SM_WRITE].config);

    if (wifi) {
        pio_gpio_init(pio0, GPIO_ENBL);
        gpio_disable_pulls(GPIO_ENBL);

        gpio_set_irq_enabled_with_callback(GPIO_RESET, GPIO_IRQ_EDGE_FALL
                                                     | GPIO_IRQ_EDGE_RISE, true, a2_reset);
        if (gpio_get(GPIO_RESET)) {
            a2_reset(GPIO_RESET, GPIO_IRQ_EDGE_RISE);
        }                                                 
    } else {
        for (uint gpio = GPIO_DEVSEL; gpio < GPIO_DEVSEL + SIZE_ENBL; gpio++) {
            pio_gpio_init(pio1, gpio);
            gpio_disable_pulls(gpio);
        }

        pio_claim_sm_mask(pio1, 0b0111);

        uint          offset;
        pio_sm_config config;

        offset = pio_add_program(pio1, &devsel_program);
        config = devsel_program_get_default_config(offset);
        pio_sm_init(pio1, SM_DEVSEL, offset, &config);

        offset = pio_add_program(pio1, &iosel_program);
        config = iosel_program_get_default_config(offset);
        pio_sm_init(pio1, SM_IOSEL, offset, &config);

        offset = pio_add_program(pio1, &iostrb_program);
        config = iostrb_program_get_default_config(offset);
        pio_sm_init(pio1, SM_IOSTRB, offset, &config);

        pio_set_sm_mask_enabled(pio1, 0b0111, true);

        a2_reset(0, GPIO_IRQ_EDGE_RISE);
    }
}

bool a2pico_wifi(void) {
    while (!ready) {
        tight_loop_contents();
    }
    return wifi;
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
        pio_sm_put(pio0, SM_SYNC, counter - 1);

        irq_set_enabled(PIO0_IRQ_0, true);
        pio_sm_set_enabled(pio0, SM_SYNC, true);
    }
}
