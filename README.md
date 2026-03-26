# A2Pico

<img src="/assets/a2pico.svg" alt="Logo" height="140" align="right">

A2Pico is about Apple II peripheral cards based on the [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/). It consists of two parts:
* Several [hardware](#hardware) reference designs
* A software library for [projects based on A2Pico](#projects-based-on-a2pico)

## Hardware

* [A2Pico TH card](https://apple2.co.uk/Products#a2pico-th-card) and [design](https://github.com/rallepalaveev/a2pico/blob/main/A2Pico.v2.6)
* [A2Pico SMD card](https://apple2.co.uk/Products#a2pico-multifunction-card) and [design](https://github.com/rallepalaveev/a2pico/blob/main/A2Pico.v2.7)
* [A2Pico SMD card](https://jcm-1.com/product/a2pico/) for the U.S.

## Firmware

### Projects based on A2Pico

* A2retroNET (https://github.com/oliverschmidt/a2retronet)
* Mouse Interface (https://github.com/oliverschmidt/mouse-interface)
* softSP (https://github.com/oliverschmidt/softsp)
* Appli-Card (https://github.com/oliverschmidt/appli-card)
* Super Serial Card (https://github.com/oliverschmidt/super-serial-card)
* Apple2-IO-RPi (https://github.com/tjboldt/Apple2-IO-RPi)
* Apple II Pi (https://github.com/oliverschmidt/apple2pi)
* Brutal Timer (https://github.com/oliverschmidt/brutal-timer)
* Bad Apple !!gs (https://github.com/oliverschmidt/bad-apple-iigs)
* A2Pico Demo (https://github.com/oliverschmidt/a2pico/tree/main/demo)

### Flashing of a Firmware

Every A2Pico firmware is flashed in the same extremly simple and foolproof way.

* A2Pico: It doesn't matter wether the card is inserted into an Apple II slot or not. Just make sure the Apple II turned off if the card is inserted.
* A2Pico2Lite: Make sure the card is not inserted into an Apple II slot.

1. Press and hold the `BOOTSEL`button on the card.
2. Connect the card to a PC.
3. Release the `BOOTSEL`button. A new drive, `RPI-RP2` or `RP2350`, will appear.
4. Copy the firmware `.uf2` file to the `RPI-RP2` or `RP2350` drive, e.g., by drag & drop.
5. Disconnect the card from the PC. Done.

## Background

### A2Pico

Soon after the introduction of the Raspberry Pi Pico in 2021 [Glenn Jones](https://github.com/a2retrosystems) and I started to experiment with directly connecting it to the Apple II slot bus. In 2022 I published a working Pico firmware on GitHub. Based on that we started to implement the _A2retroNET_ project which I presented at KanasFest 2023 (https://youtu.be/ryiH8t4yIuw).

In the meanwhile [Ralle Palaveev](https://github.com/rallepalaveev) created his own _A2Pico_ hardware for the firmware I had published before. In contrast to the hardware I presented on KansasFest, _A2Pico_ consisted completely of through-hole components which allow for very easy DIY assembly. However, at that point my Pico firmware relied on the behavior of components only available as SMD fine-pitch packages. Therefore _A2Pico_ had some functional limitations.

Considering the next steps after presenting the prototype at KansasFest I decided that my firmware should be accessible to the DIY community so I teamed up with Ralle to modify both hardware and firmware to enable full functionlity with through-hole components only and without any PLDs.

Ralle has now developed an _A2Pico_ variant with SMD components for efficient automated assembly. Both _A2Pico_ variants are 100% functionally identical.

Additionally I wanted to establish a software library that avoids duplication of low level code into the different firmware projects that all share the same hardware. The name _A2Pico_ is now used for both the common hardware and the common software.

### A2Pico2Lite

The introduction of the Raspberry Pi Pico 2 in 2024 brought true 5V tolerance, thus eliminating the need for the _A2Pico's_ transceivers. However, the _A2Pico_ also uses these transceivers to multiplex address and data lines onto the same GPIOs.  

To take advantage of the 5V tolerance and truly eliminate the transceivers, the multiplexing of address and data lines also had to be removed. Without this multiplexing, however, there is a critical shortage of GPIOs. Therefore, the _A2Pico2Lite_ does __not__ have an SD card slot. Hence the _Lite_ in its name.

In addition to the transceivers, the _A2Pico2Lite_ also eliminates the _A2Pico's_ AND gate. Therefore, it requires no ICs at all (apart from those on the Pico 2 module). This allows for the simplest possible DIY TH variant.

## Theory of Operation

### A2Pico

/DEVSEL, /IOSEL and /IOSTRB are combined to ENBL via an AND gate. A0-A7 and D0-D7 are multiplexed to the same GPIOs. D0-D7 direction is controlled by GPIO.

#### GPIO Mapping

| GPIO   | Usage    |
|:------:|:--------:|
| 0      | UART0 TX |
| 1      | UART0 RX |
| 2      | ENBL     |
| 3 - 14 | A0 - A11 |
| 3 - 10 | D0 - D7  |
| 15     | R/W      |
| 16     | $\Phi$ 1 |
| 17     | RESET    |
| 18     | /IRQ     |
| 19     | SPI0 TX  |
| 20     | SPI0 RX  |
| 21     | SPI0 CSn |
| 22     | SPI0 SCK |
| 26     | TRX0 OE  |
| 27     | TRX1 OE  |
| 28     | TRX1 DIR |

There are four PIO state machines: __addr__, __read__, __write__ and __sync__. The ARM core 0 is operated in a traditional way: Running from cached Flash, calling into the C library, being interrupted by the USB library, etc. However, The ARM core 1 is dedicated to interact with the __addr__, __read__ and __write__ PIO state machines. Therefore it runs from RAM, calls only inline functions and is never interrupted.

On the falling edge of ENBL, the __addr__ state machine samples lines A0-A11 plus R/W and pushes the data into its RX FIFO. In case of a 6502 write cycle, it additionally triggers the __write__ state machine. The ARM core 1 waits on that FIFO, decodes the address parts and branches based on R/W.

In case of a 6502 write cycle, the __write__ state machine samples lines D0-D7 ~300ns later and pushes the byte into its RX FIFO. By then, the ARM core 1 waits on that FIFO and processes the byte.

In case of a 6502 read cycle, it's up to the ARM core 1 code to produce a byte in time for the 6502 to pick it up. As soon as it has done so, it pushes the byte into the __read__ state machine TX FIFO. That state machine waits on its TX FIFO and drives out the byte to the lines D0-D7 until the rising edge of ENBL.

### A2Pico2Lite

#### GPIO Mapping

| GPIO    | Usage    |
|:-------:|:--------:|
| 0       | /IRQ     |
| 1       | $\Phi$ 0 |
| 2 - 13  | A0 - A11 |
| 14 - 21 | D0 - D7  |
| 22      | R/W      |
| 26      | /DEVSEL  |
| 27      | /IOSEL   |
| 28      | /IOSTRB  |

There are seven PIO state machines: __devsel__, __iosel__, __iostrb__, __addr__, __read__, __write__ and __sync__. The ARM core 0 is operated in a traditional way: Running from cached Flash, calling into the C library, being interrupted by the USB library, etc. However, The ARM core 1 is dedicated to interact with the __addr__, __read__ and __write__ PIO state machines. Therefore it runs from RAM, calls only inline functions and is never interrupted.

On the falling edge of /DEVSEL, /IOSEL or /IOSTRB, the __devsel__, __iosel__ or __iostrb__ state machine triggers the __addr__ state machine. The __addr__ state machine samples lines A0-A11, D0-D7 plus R/W and pushes the data into its RX FIFO. In case of a 6502 write cycle, it additionally triggers the __write__ state machine. The ARM core 1 waits on that FIFO, decodes the address parts and branches based on R/W.

In case of a 6502 write cycle, the __write__ state machine samples lines D0-D7 ~300ns later again and pushes the byte into its RX FIFO. By then, the ARM core 1 waits on that FIFO and processes the byte.

In case of a 6502 read cycle, it's up to the ARM core 1 code to produce a byte in time for the 6502 to pick it up. As soon as it has done so, it pushes the byte into the __read__ state machine TX FIFO. That state machine waits on its TX FIFO and drives out the byte to the lines D0-D7 until /DEVSEL, /IOSEL and /IOSTRB are all high.
