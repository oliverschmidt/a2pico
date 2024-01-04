# A2Pico

A2Pico is about Apple II peripheral cards based on the [Raspberry Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/). It consists of two parts:
* Several [hardware](#a2pico-hardware) reference designs
* A [software](#a2pico-software) library for projects based on A2Pico

## A2Pico Hardware

### Theory Of Operation

/DEVSEL, /IOSEL and /IOSTRB are combined to ENBL via an AND gate.

There are three PIO state machines: __enable__, __read__ and __write__. The ARM core 0 is operated in a traditional way: Running from cached Flash, calling into the
C library, being interrupted by the USB library, etc. However, The ARM core 1 is dedicated to interact with the three PIO state machines. Therefore it runs from RAM,
calls only inline functions and is never interrupted.

On the falling edge of ENBL, the the __enable__ state machine samples lines A0-A11 plus R/W and pushes the data into its RX FIFO. In case of a 6502 write cycle, it
additionally triggers the __write__ state machine. The ARM core 1 waits on that FIFO, decodes the address parts and branches based on R/W.

In case of a 6502 write cycle, the __write__ state machine samples lines D0-D7 ~300ns later and pushes the byte into its RX FIFO. By then, the ARM core 1 waits on
that FIFO and processes the byte.

In case of a 6502 read cycle, it's up to the ARM core 1 code to produce a byte in time for the 6502 to pick it up. As soon as it has done so, it pushes the byte
into the __read__ state machine TX FIFO. That state machine waits on its TX FIFO and drives out the byte to the lines D0-D7 until the rising edge of ENBL.

### GPIO Mapping

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
| 18     | ! IRQ    |
| 19     | SPI0 TX  |
| 20     | SPI0 RX  |
| 21     | SPI0 CSn |
| 22     | SPI0 SCK |
| 26     | TRX0 OE  |
| 27     | TRX1 OE  |
| 28     | TRX1 DIR |

## A2Pico Software

### Projects based on A2Pico

* Apple2-IO-RPi (https://github.com/oliverschmidt/apple2-io-rpi)
* Apple II Pi (https://github.com/oliverschmidt/apple2pi)
* Grappler+ (https://github.com/oliverschmidt/grappler-plus)
* A2Pico Demo (https://github.com/oliverschmidt/a2pico/tree/main/demo)
