# eepromprogrammer
A simple program for AVR MCU to read and write EEPROM chip (24LC256) from PC through FTDI cable.

The program uses the hardware UART (PC communication) and does software I2C (EEPROM communication).

I opted for minimalist protocol for read and write from PC, which is described below (Baud rate = 14.4k).

#### check if AVR is talking back to PC

```
Tx: 0x10
Rx: 0xd0    - something is wrong if this byte does not come back
```

#### read a page (page size == 64 bytes, address size = 2 bytes) 

```
Tx: 0x30
Tx: (high address byte)
Tx: (low address byte)
Tx: (size:   0 < size <= 64)
Rx: (size bytes)
```

#### write a page page size == 64 bytes, address size = 2 bytes) 

```
Tx: 0x20
Tx: (high address byte)
Tx: (low address byte)
Tx: (size:   0 < size <= 64)
Tx: (size bytes data)
Rx: 0xd1  - confirming write op succeeded
```

I used ATtiny4313 and current build configuration makes this assumption. Should not be very difficult to support any AVR chip type with some modification.
