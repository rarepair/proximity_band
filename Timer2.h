#ifndef TIMER2_H
#define	TIMER2_H

#define TMR2_POSTSCALER_16 0x0F
#define TMR2_POSTSCALER_15 0x0E
#define TMR2_POSTSCALER_14 0x0D
#define TMR2_POSTSCALER_13 0x0C
#define TMR2_POSTSCALER_12 0x0B
#define TMR2_POSTSCALER_11 0x0A
#define TMR2_POSTSCALER_10 0x09
#define TMR2_POSTSCALER_9 0x08
#define TMR2_POSTSCALER_8 0x07
#define TMR2_POSTSCALER_7 0x06
#define TMR2_POSTSCALER_6 0x05
#define TMR2_POSTSCALER_5 0x04
#define TMR2_POSTSCALER_4 0x03
#define TMR2_POSTSCALER_3 0x02
#define TMR2_POSTSCALER_2 0x01
#define TMR2_POSTSCALER_1 0x00
#define TMR2_PRESCALER_64 0x03
#define TMR2_PRESCALER_16 0x02
#define TMR2_PRESCALER_4 0x01
#define TMR2_PRESCALER_1 0x00

struct TIMER2
{
    unsigned char TMR2_postscaler;
    unsigned char TMR2_prescaler;
    unsigned char TMR2_maxcount;
};

void TMR2_init(struct TIMER2 dev);
void TMR2_enable(char state);

#endif	/* TIMER2_H */

