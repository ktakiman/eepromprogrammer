# eepromprogrammer
A simple program for AVR MCU to read and write EEPROM chip (24LC256) from PC through FTDI cable.

The program uses the hardware UART (PC communication) and does software I2C (EEPROM communication).

I used ATtiny4313 and current build configuration makes this assumption. Should not be very difficult to support any AVR chip type with some modification.
