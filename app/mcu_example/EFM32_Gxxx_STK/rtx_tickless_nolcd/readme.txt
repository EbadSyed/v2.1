Keil RTX RTOS - tick-less example with LCD off

This example is modification of rtx_tickless to demonstrate ultra low power consumption 
of Gecko processors in connection with RTX RTOS. Comparing to previous example it has LCD
turned off. Low frequency crystal oscillator was disabled and low frequency RC oscillator
used instead to lower energy consumption even more.


This example project uses the Keil RTX RTOS, and gives a basic demonstration
of using two tasks; one sender task generating numbers every 500ms and one receiver task.
The RTX is configured in tick-less mode, going into EM2 when no tasks are active. This 
example is intended as a skeleton for new projects using Keil RTX for energy aware 
applications. The sleep behavior of the system can be observed using Energy Profiler 
in Simplicity Studio.

Board:  Silicon Labs EFM32_Gxx_STK
Device: EFM32G890F128
