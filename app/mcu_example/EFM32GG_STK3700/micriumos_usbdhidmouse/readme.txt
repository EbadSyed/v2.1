MicriumOS USB device hid mouse example.

This example shows how to use the MicriumOS USB device stack with the
USB peripheral on the EFM32GG starter kit. This example will emulate
the behaviour of a USB mouse and move the mouse ponter up-left and
down-right periodicly when connected to a pc.

The output from the example application can be found by connecting a 
terminal to the VCOM port.

Micrium OS Support SEGGER SystemView to view the runtime behavior or a system.
SystemView Trace is enabled by default and can be disabled by changing the 
OS_CFG_TRACE_EN configuration inside the os_cfg.h file. SystemView can be used
to inspect the runtime behaviour of this example, it will give a nice overview
of the tasks and interrupts in the application. SystemView can be downloaded 
from https://www.segger.com/systemview.html

Board:  Silicon Labs EFM32GG_STK3700 Starter Kit
Device: EFM32GG990F1024
