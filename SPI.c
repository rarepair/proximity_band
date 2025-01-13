#include <xc.h>
#include "SPI.h"
#include "main.h"

void SPI_init(struct SPI dev)
{
    unsigned char SSP1CON1_b = 0b00000000;
    unsigned char SSP1STAT_b = 0b00000000;
    
    switch (dev.idleState)
    {
        /*case SPI_CLKIDLE_HIGH:
            SSP1CON1_b |= 0b00010000;
            break;*/
        case SPI_CLKIDLE_LOW:
            break;
        default:
            return;        
    }
    
    switch (dev.clockMode)
    {
        /*case SPI_CLKSRC_T2MATCH2:
            SSP1CON1_b |= 0b00000011;
            break;
        case SPI_CLKSRC_FOSC64:
            SSP1CON1_b |= 0b00000010;
            break;
        case SPI_CLKSRC_FOSC16:
            SSP1CON1_b |= 0b00000001;
            break;*/
        case SPI_CLKSRC_FOSC4:
            break;
        default:
            return;        
    }

    switch (dev.dataSampleMode)
    {
        /*case SPI_SAMPLEMODE_END:
            SSP1STAT_b |= 0b10000000;
            break;*/
        case SPI_SAMPLEMODE_MID:
            break;
        default:
            return;        
    }
    
    switch (dev.transmitMode)
    {
        case SPI_TXCLKEDGE_ACTIVETOIDLE:
            SSP1STAT_b |= 0b01000000;
            break;
        /*case SPI_TXCLKEDGE_IDLETOACTIVE:
            break;*/
        default:
            return;        
    }
    
    SSP1CON1 = SSP1CON1_b;
    SSP1STAT = SSP1STAT_b;
}

void SPI_enable(char state)
{
    if (state == FALSE)
        SSP1CON1 &= 0b11011111;
    else
        SSP1CON1 |= 0b00100000;
}

void SPI_writeByte(unsigned char byte, struct SPI dev)
{
    //There's no way to check if a SPI byte has fully shifted out of SSPBUF, so
    //we have to just add a delay in this function to prevent write collisions in
    //the SSPBUF register. 
    //byteTxTime in seconds = (Prescaler*8)/Fosc
    *(dev.chipSelectPort) &= ~(0x1u << dev.chipSelectBit);
    if (dev.endianness == SPI_LSBFIRST)
        SSPBUF = reverseByte(byte);         //write byte to SSPBUF register
    else
        SSPBUF = byte;
    __delay_us(1);                          //Wait for byte to be shifted out
    *(dev.chipSelectPort) |= (0x1 << dev.chipSelectBit);
}

static unsigned char reverseByte(unsigned char byte)
{
    static const unsigned char lookupTable[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
    };

    //Select first half of byte, shift left, and reverse nibble in mini table 
    //above. OR this value to the other nibble, shifted right, and reversed.
    return (lookupTable[byte & 0b1111] << 4) | lookupTable[byte >> 4];
}