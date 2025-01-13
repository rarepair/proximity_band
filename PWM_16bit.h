#ifndef PWM_16BIT_H
#define	PWM_16BIT_H

#define TMR1_CLKSRC_LFINTOSC 0
#define TMR1_CLKSRC_EXTERN 1
#define TMR1_CLKSRC_FOSC 2
#define TMR1_CLKSRC_FOSC4 3
#define TMR1_CLKPRESCALE_8 0
#define TMR1_CLKPRESCALE_4 1
#define TMR1_CLKPRESCALE_2 2
#define TMR1_CLKPRESCALE_1 3
#define TMR1_LPOSC_EN 0
#define TMR1_LPOSC_DIS 1
#define TMR1_ASYNC_SYNCDIS 0
#define TMR1_ASYNC_SYNCEN 1 
#define CCP1_MODE_GENSOFTINTERRUPT 0
#define CCP1_MODE_CLRONCOMPAREMATCH 1
#define CCP1_MODE_SETONCOMPAREMATCH 2

//We have to remember this because the interrupt requires it to work properly
unsigned char CCP1_mode;

struct PWM16 
{
    unsigned char TMR1_clksrc;
    unsigned char TMR1_clkprescale;
    unsigned char TMR1_lposc;
    unsigned char TMR1_async;
    unsigned char CCP1_mode;    
};

void PWM16_init(struct PWM16 dev);
void PWM16_enable(char state);
void PWM16_setDuty(unsigned int duty);

#endif	/* PWM_16BIT_H */

