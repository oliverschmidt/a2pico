/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// -----------------------------------------------------
// NOTE: THIS HEADER IS ALSO INCLUDED BY ASSEMBLER SO
//       SHOULD ONLY CONSIST OF PREPROCESSOR DIRECTIVES
// -----------------------------------------------------

#ifndef _BOARDS_A2PICO_H
#define _BOARDS_A2PICO_H

pico_board_cmake_set(PICO_PLATFORM, rp2350)
pico_board_cmake_set(PICO_CYW43_SUPPORTED, 1)

#define A2PICO2

// --- RP2350 VARIANT ---
// This means RP2350B
#define PICO_RP2350A 0

// --- CYW43 ---
#define CYW43_PIN_WL_DYNAMIC 0
#define CYW43_DEFAULT_PIN_WL_REG_ON 23
#define CYW43_DEFAULT_PIN_WL_DATA_OUT 24
#define CYW43_DEFAULT_PIN_WL_DATA_IN 24
#define CYW43_DEFAULT_PIN_WL_HOST_WAKE 24
#define CYW43_DEFAULT_PIN_WL_CS 25
#define CYW43_DEFAULT_PIN_WL_CLOCK 29

// --- SD ---
#define PICO_SD_CLK_PIN 34
#define PICO_SD_CMD_PIN 35
#define PICO_SD_DAT0_PIN 36
#define PICO_SD_DAT1_PIN 37
#define PICO_SD_DAT2_PIN 38
#define PICO_SD_DAT3_PIN 39
#define PICO_SD_CARD_DETECT_PIN 40
#define PICO_SD_DAT_PIN_INCREMENT 1
#define PICO_SD_DAT_PIN_COUNT 4

// --- ETH ---
#define PICO_ETH_SPI 1
#define PICO_ETH_RST_PIN 42
#define PICO_ETH_INT_PIN 43
#define PICO_ETH_SPI_RX_PIN 44
#define PICO_ETH_SPI_CSN_PIN 45
#define PICO_ETH_SPI_SCK_PIN 46
#define PICO_ETH_SPI_TX_PIN 47

// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#define PICO_FLASH_SPI_CLKDIV 2
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (2 * 1024 * 1024))
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)

pico_board_cmake_set_default(PICO_RP2350_A2_SUPPORTED, 1)
#define PICO_RP2350_A2_SUPPORTED 1

#endif
