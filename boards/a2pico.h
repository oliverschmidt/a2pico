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

pico_board_cmake_set(PICO_PLATFORM, rp2040)

#define A2PICO

// --- UART ---
#define PICO_DEFAULT_UART 0
#define PICO_DEFAULT_UART_TX_PIN 0
#define PICO_DEFAULT_UART_RX_PIN 1

// --- LED ---
#define PICO_DEFAULT_LED_PIN 25

// --- SPI ---
#define PICO_DEFAULT_SPI 0
#define PICO_DEFAULT_SPI_TX_PIN 19
#define PICO_DEFAULT_SPI_RX_PIN 20
#define PICO_DEFAULT_SPI_CSN_PIN 21
#define PICO_DEFAULT_SPI_SCK_PIN 22

// --- FLASH ---
#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1
#define PICO_FLASH_SPI_CLKDIV 2
pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (2 * 1024 * 1024))
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)

#define PICO_SMPS_MODE_PIN 23
#define PICO_VBUS_PIN 24
#define PICO_VSYS_PIN 29

#define PICO_RP2040_B0_SUPPORTED 1

#endif
