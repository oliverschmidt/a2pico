# Demo

The program demostrates how the Pico can interact directly with the Apple II 1MHz slot bus. The program emulates both a slot ROM ($Cn00-$CnFF) as well a an expansion
ROM ($C800-$CFFF). Additionally it emulates a simple UART-style interface ($C0n0) with two status bits for ready-to-send and ready-to-receive ($C0n1).

The program comes with some 6502 firmware, which makes use of the three emulated items to interact with a terminal emulator connected via the Pico USB port.

The only additional hardware strictly necessary is an AND gate with (at least) 3 inputs. However, it is strongly recommended to use a bus switch with (at least)
24 bits for level shifting between the 5V of the A2 and the 3.3V of the Pico.

## Usage

1. Flash the [demo.uf2](https://github.com/a2retrosystems/A2retroNET/releases/latest/download/demo.uf2) file to the Pico as usual, but don't remove the USB cable
from the Pico.

2. Turn off the Apple II.

3. Connect the Pico to the Apple II and connected the USB cable to your USB host.

4. Turn on the Apple II.

The program creates an ACM CDC serial port on the USB host. It deliberately stops there and waits for a program to connect to the serial port on the USB host.

5. Connect a terminal emulator (e.g. [PuTTY](https://www.putty.org/)) to the serial port @ 115200 bits/s.
 
After the terminal has asserted DTR (which most of them do implicitly), the program continues to start up and sends a banner to the terminal.

6. Get into the Apple II Monitor and enter `Cn00` (with n being the slot the Pico is connected to). Now the Pico LED comes on. Enter `CFFF` and the LED comes off.

7. Enter `Cn00L` to see the firmware code of the ROM emulated by the Pico.

8. Enter `Cn00G` to run the firmware. A banner is displayed on the Apple II display.
 
9. Enter chars on the Apple II and the terminal. They are displayed on the counterside.

10. Press `ESC` on the Apple II to quit to the monitor. After eight further chars entered on the terminal, the program sends a BELL to the terminal.

11. Enter `Cn00G` again and the eight chars are displayed.

12. Quit the terminal emulator (or de-assert DTR, if possible). After eight further chars entered on the Apple II, the Apple II firmware makes the speaker beep.

13. Connect the terminal emulator again (or assert DTR again) and the eight chars are sent to it.

14. Press `ESC` on the terminal. An interrupt handler on the Apple II inverts the character in the lower right corner of the screen.

15. Press `Ctrl-Reset` on the Apple II. The string `RES` is displayed on the terminal.

## Theory of operation

/DEVSEL, /IOSEL and /IOSTRB are supposed to combined to ENBL via an AND gate.

There are three PIO state machines: __enable__, __read__ and __write__. The ARM core 0 is operated in a traditional way: Running from cached Flash, calling into the
C library, being interrupted by the USB library, etc. However, The ARM core 1 is dedicated to interact with the three PIO state machines. Therefore it runs from RAM,
calls only inline functions and is never interrupted.

On the falling edge of ENBL, the the __enable__ state machine samples lines A0-A11 plus R/W and pushes the data into its RX FIFO. In case of a 6502 write cycle, it
additionally triggers the __write__ state machine. The ARM core 1 waits on that FIFO, decodes the address parts and branches based on R/W.

In case of a 6502 write cycle, the __write__ state machine samples lines D0-D7 ~300ns later and pushes the byte into its RX FIFO. By then, the ARM core 1 waits on
that FIFO and processes the byte.

In case of a 6502 read cycle, it's up to the ARM core 1 code to produce a byte in time for the 6502 to pick it up. As soon as it has done so, it pushes the byte
into the __read__ state machine TX FIFO. That state machine waits on its TX FIFO and drives out the byte to the lines D0-D7 until the rising edge of ENBL.

## GPIO port considerations

The Pico has two GPIO port ranges avialable for external connections: GPIO0 - GPIO22 (= 23 ports) and GPIO26 - GPIO28 (= 3 ports). A PIO state machine can sample only
consecutive pins. With the __write__ state machine sampling D0-D7, this means that there are only 15 GPIO ports left for state machine sampling.

So, the Pico simply doesn't have enough GPIO ports to sample A0-A15 plus R/W (= 17 ports). Therefore /DEVSEL, /IOSEL and /IOSTRB have to be used. However, a PIO state
machine can only wait on a single pin. Using multiple PIO state machines is no viable alternative as an ARM core can only wait on a single state machine FIFO. The way
out is to externally combine /DEVSEL, /IOSEL and /IOSTRB into a single ENBL line. This "&Phi;0 replacement" allows to reduce the state machine sampling to A0-A11
necessary for supporting an expansion ROM plus R/W (= 13 ports).

The two spare GPIO ports in the GPIO0 - GPIO22 range are especially valuable as the GPIO26 - GPIO28 range can't be used by a UART, only by an I2C controller.

This is the actual pinout for the demo program - the brackets mark usage ideas:
| GPIO    | Usage     |
|:--------|:----------|
| 0       | (UART TX) |
| 1       | (UART RX) |
| 2 - 13  | A0 - A11  |
| 14      | R/W       |
| 15 - 22 | D0 - D7   |
| 26      | ENBL      |
| 27      | IRQ       |
| 28      | RES       |

## More GPIO port considerations

An SPI controller (or a UART with H/W flow control) requires four spare GPIO ports in the GPIO0 - GPIO22 range. This requirement can be accommodated by omitting the
expansion ROM support. With only the slot ROM, A0-A7 are sufficient. However, in order to still be able to distinguish between /DEVSEL and /IOSEL, one of those two
needs to be connected to a GPIO port - in addition to combining them both to the ENBL line via an AND gate.

This is a potential pinout w/o expansion ROM - the brackets mark usage ideas:
| GPIO    | Usage     |
|:--------|:----------|
| 0       | (SPI RX)  |
| 1       | (SPI CSn) |
| 2       | (SPI SCK) |
| 3       | (SPI TX)  |
| 4       | (NMI)     |
| 5 - 12  | A0 - A7   |
| 13      | DEVSEL    |
| 14      | R/W       |
| 15 - 22 | D0 - D7   |
| 26      | ENBL      |
| 27      | (IRQ)     |
| 28      | (RES)     |

With this setup, the slot ROM firmware is identical for all slots. Therefore, it needs to use the traditional method from the _Apple II Reference Manual_, pages
81/82 to work in all slots.

Banking of multiple slot ROM pages via a bank register in the $C0n0 - $C0nF slot I/O space is an obvious way to mitigate potential firmware space shortage. Doing so
is a matter of adding a single line of C code to the ARM core 1 code.

## Building

1. Install __cc65__ according to [Getting Started](https://cc65.github.io/getting-started.html).

2. Create __firmware.bin__ with `cl65 firmware.S -C firmware.cfg -o firmware.bin`.

3. Build __demo.uf2__ according to [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).
