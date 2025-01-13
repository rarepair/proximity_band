#ifndef SPI_H
#define	SPI_H

#define SPI_CLKIDLE_HIGH 1
#define SPI_CLKIDLE_LOW 0
#define SPI_CLKSRC_T2MATCH2 3
#define SPI_CLKSRC_FOSC64 2
#define SPI_CLKSRC_FOSC16 1
#define SPI_CLKSRC_FOSC4 0
#define SPI_SAMPLEMODE_END 1
#define SPI_SAMPLEMODE_MID 0
#define SPI_TXCLKEDGE_ACTIVETOIDLE 1
#define SPI_TXCLKEDGE_IDLETOACTIVE 0
#define SPI_LSBFIRST 1
#define SPI_MSBFIRST 0

struct SPI 
{
    unsigned char idleState;
    unsigned char clockMode;
    unsigned char dataSampleMode;
    unsigned char transmitMode;
    unsigned char endianness;
    unsigned char *chipSelectPort;
    unsigned char chipSelectBit;
};

void SPI_init(struct SPI dev);
void SPI_enable(char state);
void SPI_writeByte(unsigned char byte, struct SPI dev);
static unsigned char reverseByte(unsigned char byte);

#endif	/* SPI_H */

