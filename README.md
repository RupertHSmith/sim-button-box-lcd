# sim-button-box-lcd
This project has two main components:-

A button box which is connected as a standard HID USB joystick device with no drivers required. Buttons can be mapped ingame as required.

An LCD display which is controlled by a driver application on the PC. This displays game specific information such as tyre temperatures and laptimes. 

The microcontroller used is an Atmel AT90USB1286. The LUFA library has been used for the USB programming and the microcontroller code is written purely in C.
