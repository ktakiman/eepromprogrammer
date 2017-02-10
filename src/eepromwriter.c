#include <avr/io.h>
#include <avr/power.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "../avrlib/i2c.h"

#define CMD_STATUS  0x10
#define CMD_WRITE   0x20
#define CMD_READ    0x30

#define STATUS_IDLE           0xa0
#define STATUS_WRITE          0xa1
#define STATUS_READ           0xa2

#define WRITE_EXPECT_HIGH_ADDR  0xb0
#define WRITE_EXPECT_LOW_ADDR   0xb1
#define WRITE_EXPECT_SIZE       0xb2
#define WRITE_EXPECT_DATA       0xb3

#define READ_EXPECT_HIGH_ADDR   0xc0
#define READ_EXPECT_LOW_ADDR    0xc1
#define READ_EXPECT_SIZE        0xc2

#define NOTIFY_IDLE             0xd0
#define NOTIFY_WRITE_COMPLETE   0xd1

volatile uint8_t gMode = STATUS_IDLE;

volatile uint8_t gWritePhase;
//uint8_t gWriteAddrHigh;
//uint8_t gWriteAddrLow;
volatile uint16_t gWriteAddr;
volatile uint8_t gWriteSize;
volatile uint8_t gWriteBufPos;

volatile uint8_t gReadPhase;
//uint8_t gReadAddrHigh;
//uint8_t gReadAddrLow;
volatile uint16_t gReadAddr;
volatile uint8_t gReadSize;

#define BUF_SZ 64
uint8_t gBuf[BUF_SZ];

// ----------------------------------------------------------------------------
void serialWrite(uint8_t data)
{
    while (!(UCSRA & (1 << UDRE))); // wait until last transmission is complete
    UDR = data;
}
// ----------------------------------------------------------------------------
void serialWriteBuf(uint8_t* pBuf, uint8_t sz)
{
    uint8_t i;
    for (i = 0; i < sz; i++)
    {
        serialWrite(*(pBuf + i));
    }
}
// ----------------------------------------------------------------------------
uint8_t serialRead()
{
    while (!(UCSRA & (1 << RXC)));
    return UDR;    
}
// ----------------------------------------------------------------------------
void i2cDump(uint8_t* pText)
{
}
// ----------------------------------------------------------------------------
int main(void) {
    clock_prescale_set(clock_div_1);

    DDRB |= (1 << PINB3);
    PORTB |= (1 << PINB3);

    UCSRB |= (1 << RXEN | 1 << TXEN); // enable UART RX and TX
    UCSRC |= (1 << UCSZ1 | 1 << UCSZ0); // character size = 8, 1 stop bit
    UBRRL = 34; // baud rate 14.4k
    
    // while (1)
    // {
        // volatile uint8_t data = serialRead();
        // serialWrite(data);
    // }
    
    struct I2C i2c;
    i2c.pDDR = &DDRB;
    i2c.pPort = &PORTB;
    i2c.pPin = &PINB;
    i2c.sda = PINB0;
    i2c.scl = PINB1;
    i2c.pDumpCallback = i2cDump;    
    
    _i2c_init(&i2c);
    
    while (1)
    {
        uint8_t data = serialRead();
    
        if (gMode == STATUS_IDLE)
        {
            switch (data)
            {
                case CMD_STATUS: 
                    serialWrite(NOTIFY_IDLE); 
                    break;
                case CMD_WRITE: 
                    gMode = STATUS_WRITE; 
                    gWritePhase = WRITE_EXPECT_HIGH_ADDR;
                    gWriteBufPos = 0;
                    PORTB &= ~(1 << PINB3);
                    break;
                case CMD_READ:
                    gMode = STATUS_READ;
                    gReadPhase = READ_EXPECT_HIGH_ADDR;
                    PORTB &= ~(1 << PINB3);
                    break;
                default:
                    break;
            }
        }
        else if (gMode == STATUS_WRITE)
        {
            switch (gWritePhase)
            {
                case WRITE_EXPECT_HIGH_ADDR:
                    //gWriteAddrHigh = data;
                    gWriteAddr = data << 8;
                    gWritePhase = WRITE_EXPECT_LOW_ADDR;
                    break;
                case WRITE_EXPECT_LOW_ADDR:
                    //gWriteAddrLow = data;
                    gWriteAddr |= data;
                    gWritePhase = WRITE_EXPECT_SIZE;
                    break;  
                case WRITE_EXPECT_SIZE:
                    gWriteSize = data < 64 ? data : 64;
                    gWritePhase = WRITE_EXPECT_DATA;
                    break;
                case WRITE_EXPECT_DATA:
                    gBuf[gWriteBufPos] = data;
                    gWriteBufPos++;
                    if (gWriteBufPos == gWriteSize || gWriteBufPos == 64)
                    {
                        //_eeprom_write(&i2c, gWriteAddrHigh, gWriteAddrLow, gBuf, 0, gWriteSize, 1);
                        _eeprom_write(&i2c, gWriteAddr, gBuf, 0, gWriteSize, 1);
                        serialWrite(NOTIFY_WRITE_COMPLETE);
                        gMode = STATUS_IDLE;
                        PORTB |= (1 << PINB3);
                    }
                    break;
                default:
                    break; 
            }
        }
        else if (gMode == STATUS_READ)
        {
            switch (gReadPhase)
            {
                case READ_EXPECT_HIGH_ADDR:
                    //gReadAddrHigh = data;
                    gReadAddr = data << 8;
                    gReadPhase = READ_EXPECT_LOW_ADDR;
                    break;
                case READ_EXPECT_LOW_ADDR:
                    //gReadAddrLow = data;
                    gReadAddr |= data;
                    gReadPhase = READ_EXPECT_SIZE;
                    break;  
                case READ_EXPECT_SIZE:
                    gReadSize = data < BUF_SZ ? data : BUF_SZ;
                    //_eeprom_read(&i2c, gReadAddrHigh, gReadAddrLow, gBuf, 0, gReadSize);
                    _eeprom_read(&i2c, gReadAddr, gBuf, 0, gReadSize);
                    serialWriteBuf((uint8_t*)gBuf, gReadSize);
                    gMode = STATUS_IDLE;
                    PORTB |= (1 << PINB3);
                    break;
                default:
                    break; 
            }      
        }
        else
        {
            serialWrite(gMode);
            _delay_ms(1000);
        }
    }
}






