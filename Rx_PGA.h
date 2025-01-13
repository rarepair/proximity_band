#ifndef RX_PGA_H
#define	RX_PGA_H

#define RXGAIN025 0x00
#define RXGAIN1 1
#define RXGAIN10 10
#define RXGAIN20 20
#define RXGAIN30 30
#define RXGAIN40 40
#define RXGAIN60 60
#define RXGAIN80 80
#define RXGAIN120 120
#define RXGAIN157 160               //Actual gain is 157 but approximate as 160
#define VOSCALLIMIT 25              //Limit when doing VOS cal. If ADC voltage exceeds this, stop cal and use the previous setting (mV)

void RxPGA_init(struct SPI dev);
void RxPGA_enable(char state);
void RxPGA_setGain(unsigned char gain);
static void RxPGA_enableMeasureMode(char state);
static void RxPGA_setTrimVOS(unsigned char trimCode);
unsigned char RxPGA_calVOS(void);

#endif	/* RX_PGA_H */

