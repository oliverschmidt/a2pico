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

#include <string.h>
#include <stdlib.h>
#include <hardware/sync.h>
#include <hardware/flash.h>

#include "a2pico.h"

#if PICO_RP2350
#define FLASH_ADDRESS (XIP_NOCACHE_NOALLOC_NOTRANSLATE_BASE)
#else
#define FLASH_ADDRESS (XIP_BASE)
#endif

#if PICO_RP2350_A2_SUPPORTED
#define FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE - FLASH_SECTOR_SIZE)
#else
#define FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#endif

#define MAGIC 0x5069636F  // "Pico"

struct flash {
    uint32_t size;
    uint32_t magic;
    char wifi_ssid  [32 + 1];
    char wifi_passwd[63 + 1];
};

static_assert(sizeof(struct flash) <= FLASH_PAGE_SIZE, "struct flash too big");

bool __no_inline_not_in_flash_func(a2pico_save_wifi_credentials)(const char *ssid,
                                                                 const char *passwd) {
    struct flash *flash_data = malloc(FLASH_PAGE_SIZE);
    if (flash_data == NULL) {
        return false;
    }

    flash_data->size  = sizeof(*flash_data);
    flash_data->magic = MAGIC;
    strcpy(flash_data->wifi_ssid,   ssid);
    strcpy(flash_data->wifi_passwd, passwd);

    uint32_t save = save_and_disable_interrupts();
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_OFFSET, (void*)flash_data, FLASH_PAGE_SIZE);
    restore_interrupts(save);

    free(flash_data);
    return true;
}

bool a2pico_load_wifi_credentials(const char **ssid, const char **passwd) {
    const struct flash *flash_data = (struct flash *)(FLASH_ADDRESS + FLASH_OFFSET);

    if (flash_data->size  != sizeof(*flash_data) ||
        flash_data->magic != MAGIC) {
        *ssid   = NULL;
        *passwd = NULL;
        return false;
    }

    *ssid   = flash_data->wifi_ssid;
    *passwd = flash_data->wifi_passwd;
    return true;
}

void __no_inline_not_in_flash_func(a2pico_delete_wifi_credentials)(void) {
    uint32_t save = save_and_disable_interrupts();
    flash_range_erase(FLASH_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(save);
}

uint32_t a2pico_flashsize(void) {
    return FLASH_OFFSET;
}
