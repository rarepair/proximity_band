#ifndef ADC_H
#define	ADC_H

#define ADC_CLKSRC_FOSC64 5
#define ADC_CLKSRC_FOSC32 4
#define ADC_CLKSRC_FOSC16 3
#define ADC_CLKSRC_FOSC8 2
#define ADC_CLKSRC_FOSC4 1
#define ADC_CLKSRC_FOSC2 0
#define ADC_INPUT_FVRBUF1OUT 14
#define ADC_INPUT_DACOUT 13
#define ADC_INPUT_TEMPSENS 12
#define ADC_INPUT_AN11 11
#define ADC_INPUT_AN10 10
#define ADC_INPUT_AN9 9
#define ADC_INPUT_AN8 8
#define ADC_INPUT_AN7 7
#define ADC_INPUT_AN6 6
#define ADC_INPUT_AN5 5
#define ADC_INPUT_AN4 4
#define ADC_INPUT_AN3 3
#define ADC_INPUT_AN2 2
#define ADC_INPUT_AN1 1
#define ADC_INPUT_AN0 0
#define ADC_RESULT_RIGHTJUST 1
#define ADC_RESULT_LEFTJUST 0
#define ADC_NEGVREF_EXT 1
#define ADC_NEGVREF_VSS 0
#define ADC_POSVREF_FVR 2
#define ADC_POSVREF_EXT 1
#define ADC_POSVREF_VDD 0

struct ADC 
{
    unsigned char ADC_resultfmt;
    unsigned char ADC_clksrc;
    unsigned char ADC_posvrefsrc;
    unsigned char ADC_negvrefsrc;
    unsigned char ADC_chansrc;
};

void ADC_init(struct ADC dev);
void ADC_enable(char state);
unsigned int ADC_convert(void);

#endif	/* ADC_H */

