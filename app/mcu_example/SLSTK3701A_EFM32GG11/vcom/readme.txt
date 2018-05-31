USART/LEUART communication example.

This example project uses the EFM32 CMSIS and EMLIB, to demonstrate the
use of USART and LEUART communication on the SLSTK3701A Starter Kit, as well
as using the virtual COM port on the Starter Kit.

For the default communication, LEUART0 configured for 9600-8-N-1 is used.
(EXP pin 1 is GND, EXP pin 12 is LEUART0 Tx and EXP pin 14 is LEUART0 Rx).

By defining "RETARGET_VCOM" as a build option, a virtual COM port through the
USB cable is enabled. VCOM will use USART4 configured for 115200-8-N-1.

Board:  Silicon Labs SLSTK3701A Starter Kit
Device: EFM32GG11B820F2048GL192
