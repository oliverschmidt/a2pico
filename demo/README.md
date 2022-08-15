The program demostrates how the Pico can interact directly with the Apple II 1MHz slot bus. The program emulates both a slot ROM ($Cn00-$CnFF) as well a an expansion
ROM ($C800-$CFFE). Additionally it emulates a simple UART-style interface ($C0n0) with two status bits for ready-to-send and ready-to-receive ($0n1).

The program comes with some 6502 firmware, which makes use of the three emulated items to interact with a terminal emulator connected via the Pico USB port.

The only addional hardware strictly necessary is an AND gate with (at least) 3 inputs. However, it is strongly recommended to use a bus switch with (at least) 24 bits
for level shifting between the 5V of the A2 and the 3.3V of the Pico.

## Usage
Flash the demo.uf2 file to the Pico as usual, but don't remove the USB cable from the Pico. Connect the Pico to the Apple II and connected the USB cable to your USB
host.

Right after an Apple II Reset, the program creates an ACM CDC serial port on the USB host. It deliberately stops there and waits for a program to connect to the
serial port on the USB host. At that point, the Pico is still invisible for the A2.

Connect a terminal emulator (e.g. PuTTY) to the serial port @ 115200 bits/s. After the terminal has asserted DTR (which most of them do implicitly), the program
continues to start up and sends a banner to the terminal.

Get into the Apple II Monitor and enter `Cn00` (with n being the slot the Pico is connected to). Now the Pico LED comes on. Enter `CFFF` and the LED comes off. Enter
`Cn00L` to see the firmware code of the ROM emulated by the Pico.

Enter `Cn00G` to run the firmware. A banner displayed on the Apple II display. Now, every char entered on the A2 is displayed on the terminal and vice versa.

Press `ESC`on the Apple II to quit to the monitor. After 8 further chars entered on the terminal, the program sends a BELL to the terminal. Enter `Cn00G` again and
the  chars re displayed.

Quit the terminal emulator (or de-assert DTR, if possible). After 8 further chars entered on the Apple II, the Apple II firmware has the A2 speaker beep. Connect the
terminal emulator again (or assert DTR again) and the 8 chars are sent to it.

## Theory of operation

/DEVSEL, /IOSEL and /IOSTRB are supposed to combined to ENBL via an AND gate.

There are 3 PIO state machines: __enable__, __read__ and __write__. The ARM core 0 is operated in a traditional way: Running from cached Flash, calling into the C
library, being interrupted by the USB library, etc. However, The ARM core 1 is dedicated to interact with the 3 PIO state machines. Therefore it runs from RAM, calls
only inline functions and is never interrupted.

On the falling edge of ENBL, the the __enable__ state machine samples lines A0-A11 and R/W are pushes the data into its RX FIFO. In case of a 6502 write cycle, it
additionally triggers the __write__ state machine. The ARM core 1 waits on that FIFO, decodes the address parts and branches based on R/W.

In case of a 6502 write cycle, the __write__ state machine samples lines D0-D7 ~300ns later and pushes the byte into its RX FIFO. By then, the ARM core 1 waits on
that FIFO and process the byte.

In case of a 6502 read cycle, it's up to the ARM core 1 code to produce a byte in time for the 6502 to pick it up. As soon as it has done so, it pushes the byte
into the __read__ state machine TX FIFO. That state machine waits on its TX FIFO and drives out the byte to the lines D0-D7 until the rising edge of ENBL.

## GPIO port considerations

The Pico has 2 GPIO port ranges avialable for external connections: GPIO0 - GPIO22 (= 23 ports) and GPIO26 - GPIO28 (= 3 ports). A PIO state machine can sample only
consecutive pins. With the __write__ state machines sampling D0-D7, this means that there are only 15 GPIO ports left for state machine sampling.

So, the Pico simply doesn't have enough GPIO ports for A0-A15 plus &Phi;0 plus R/W. Therefore the /DEVSEL, /IOSEL and /IOSTRB have to be used. However, a PIO state
machine can only wait on a single pin. Using multiple PIO state machines is no viable alternative as an ARM core can only wait on a single state machine FIFO.
The way out is to externally combine /DEVSEL, /IOSEL and /IOSTRB into a single ENBL line. Together with the 12 address lines necessary to support an expansion ROM,
this approach requires only 13 GPIO ports.

The 2 spare GPIO ports in the GPIO0 - GPIO22 range are especially valuable as the GPIO26 - GPIO28 range can't be used by a built-in UART, only by a built-in I2C
controller.

This is the actual pinout for the demo program - the brackets mark usage ideas: 
| GPIO    | Usage     |
|:--------|:----------|
| 0       | (UART TX) |
| 1       | (UART RX) |
| 2 - 13  | A0 - A11  |
| 14      | R/W       |
| 15 - 22 | D0 - D7   |
| 26      | ENBL      |
| 27      | (IRQ)     |
| 28      | (RDY)     |

## More GPIO port considerations

A built-in SPI controller (or a built-in UART with H/W flow control) requires 4 spare GPIO ports in the GPIO0 - GPIO22 range. This requirement can be accommodated by
omitting the expansion ROM support. With only the slot ROM, A0-A7 are sufficient. However, in order to still be able to distinguish between /DEVSEL and /IOSEL, one
of those two needs to be connected to a PGIO port - in addion to combining them both to the ENBL line via an AND gate.

This is a potential pinout - the brackets mark usage ideas:
| GPIO    | Usage     |
|:--------|:----------|
| 0       | (SPI RX)  |
| 1       | (SPI CSn) |
| 2       | (SPI SCK) |
| 3       | (SPI TX)  |
| 4 - 11  | A0 - A7   |
| 12      | DEVSEL    |
| 13      | R/W       |
| 14 - 21 | D0 - D7   |
| 22      | ENBL      |
| 26      | (NMI)     |
| 27      | (IRQ)     |
| 28      | (RDY)     |

With this setup, the slot ROM firmware is identical for all slots. Therefore, it needs to use the traditional method from _Apple II Reference Manual_ page 81/82 to
work in all slots - and it needs to use the "LDA/STA $BFFF,x trick" for accessing $C0n0 - $C0nE to avoid phantom reads of the language card soft switches.

Banking of multiple slot ROM pages via a bank register in the $C0n0 - $C0nE slot I/O space is an obvious way to mitigate potential firmware space shortage. Doing so
is a matter of adding a single line of C code to the ARM core 1 code.
