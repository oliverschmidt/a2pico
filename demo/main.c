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

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>

#include <a2pico.h>

#include "board.h"

static volatile bool sync;

static void synchandler(void) {
    sync = true;
}

void main(void) {
    a2pico_init(board);

    if (a2pico_led() >= 0) {
        gpio_init(a2pico_led());
        gpio_set_dir(a2pico_led(), GPIO_OUT);
    }

    stdio_usb_init();
    while (!stdio_usb_connected()) {
    }

    printf("\n\nCopyright (c) 2022 Oliver Schmidt (https://a2retro.de/)\n\n");

    printf("Features - SD card:%s  Radio:%s  Ethernet:%s\n\n",
           a2pico_sd()    ? "true" : "false",
           a2pico_radio() ? "true" : "false",
           a2pico_eth()   ? "true" : "false");

    a2pico_synchandler(synchandler, 5000000);

    while (true) {
        if (stdio_usb_connected()) {
            if (multicore_fifo_rvalid()) {
                putchar(multicore_fifo_pop_blocking());
            }
        }

        int data = getchar_timeout_us(0);
        if (data != PICO_ERROR_TIMEOUT) {
            switch (data) {
                case 4:  // Ctrl-D
                    a2pico_delete_wifi_credentials();
                    printf(" DELETE ");
                    break;
                case 12:  // Ctrl-L
                    const char *ssid;
                    const char *passwd;
                    if (a2pico_load_wifi_credentials(&ssid, &passwd)) {
                        printf(" LOAD(\"%s\",\"%s\") ", ssid, passwd);
                    } else {
                        printf(" LOAD(N/A) ");
                    }
                    break;
                case 19:  // Ctrl-S
                    if (a2pico_save_wifi_credentials("MySSID", "MyPasswd")) {
                        printf(" SAVE(\"MySSID\",\"MyPasswd\") ");
                    } else {
                        printf(" SAVE(N/A) ");
                    }
                    break;
                case 27:  // Esc
                    a2pico_irq(true);
                    break;
                default:
                    if (multicore_fifo_wready()) {
                        multicore_fifo_push_blocking(data);
                    } else {
                        putchar('\a');
                    }
            }
        }

        if (reset) {
            reset = false;
            printf(" RESET ");
        }

        if (sync) {
            sync = false;
            printf(" SYNC ");
        }

        if (a2pico_led() >= 0) {
            gpio_put(a2pico_led(), active);
        }
    }
}
