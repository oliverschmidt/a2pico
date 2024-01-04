# Demo

The program emulates both a slot ROM ($Cn00-$CnFF) as well a an expansion ROM ($C800-$CFFE). Additionally it emulates a simple UART-style interface ($C0n0)
with two status bits for ready-to-send and ready-to-receive ($C0n1). And finally it makes use of the IRQ and RESET signals.

The program comes with some 6502 firmware, which makes use of the three emulated items to interact with a terminal emulator connected via the Pico USB port.

## Usage

1. Flash the [demo.uf2](https://github.com/oliverschmidt/a2pico/releases/latest/download/demo.uf2) file to the Pico as usual, but don't remove the USB cable
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

15. Press `Ctrl-Reset` on the Apple II. The string ` RESET ` is displayed on the terminal.
