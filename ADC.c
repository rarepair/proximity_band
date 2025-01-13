#include <xc.h>
#include "ADC.h"
#include "main.h"


void ADC_init(struct ADC dev)
{
    unsigned char ADCON1_b = 0b00000000;
    
    switch (dev.ADC_resultfmt)
    {
        case ADC_RESULT_RIGHTJUST:
            ADCON1_b |= 0b10000000;
            break;
        /*case ADC_RESULT_LEFTJUST:
            break;*/
        default:
            return;
    }
    
    switch (dev.ADC_clksrc)
    {
        /*case ADC_CLKSRC_FOSC64:
            ADCON1_b |= 0b01100000;
            break;*/
        case ADC_CLKSRC_FOSC32:
            ADCON1_b |= 0b00100000;
            break;
        /*
        case ADC_CLKSRC_FOSC16:
            ADCON1_b |= 0b01010000;
            break;
        case ADC_CLKSRC_FOSC8:
            ADCON1_b |= 0b00010000;
            break;
        case ADC_CLKSRC_FOSC4:
            ADCON1_b |= 0b01000000;
            break;
        case ADC_CLKSRC_FOSC2:
            break;
        */
        default:
            return;
    }
    
    switch (dev.ADC_negvrefsrc)
    {
        /*case ADC_NEGVREF_EXT:
            ADCON1_b |= 0b00000100;
            break;*/
        case ADC_NEGVREF_VSS:
            break;
        default:
            return;
    }

    switch (dev.ADC_posvrefsrc)
    {
        case ADC_POSVREF_FVR:
            ADCON1_b |= 0b00000011;
            break;
        /*case ADC_POSVREF_EXT:
            ADCON1_b |= 0b00000010;
            break;
        case ADC_POSVREF_VDD:
            break; */
        default:
            return;
    }
    
    switch (dev.ADC_chansrc)
    {
        /*
        case ADC_INPUT_FVRBUF1OUT:
            ADCON0 = 0b01111100;
            break;
        case ADC_INPUT_DACOUT:
            ADCON0 = 0b01111000;
            break;
        case ADC_INPUT_TEMPSENS:
            ADCON0 = 0b01110100;
            break;
        case ADC_INPUT_AN11:
            ADCON0 = 0b00101100;
            break;
        case ADC_INPUT_AN10:
            ADCON0 = 0b00101000;
            break;
        case ADC_INPUT_AN9:
            ADCON0 = 0b00100100;
            break;
        case ADC_INPUT_AN8:
            ADCON0 = 0b00100000;
            break;
        case ADC_INPUT_AN7:
            ADCON0 = 0b00011100;
            break;
        case ADC_INPUT_AN6:
            ADCON0 = 0b00011000;
            break;
        case ADC_INPUT_AN5:
            ADCON0 = 0b00010100;
            break;
        case ADC_INPUT_AN4:
            ADCON0 = 0b00010000;
            break;
        */
        case ADC_INPUT_AN3:
            ADCON0 = 0b00001100;
            break;
        /*case ADC_INPUT_AN2:
            ADCON0 = 0b00001000;
            break;
        case ADC_INPUT_AN1:
            ADCON0 = 0b00000100;
            break;
        case ADC_INPUT_AN0:
            break;*/
        default:
            return;
    }
    
    ADCON1 = ADCON1_b;
}

void ADC_enable(char state)
{
    if (state == FALSE)
        ADCON0 &= 0b11111100;                        //Disable ADC
    else
        ADCON0 |= 0b00000001;                        //Enable ADC
}

unsigned int ADC_convert(void)
{
    unsigned int integerResult = 0;
    ADCON0 |= 0b00000010;                        //Start conversion
    while (ADCON0bits.ADGO != 0) {}              //Wait for ADC conversion to complete, indicated by GO/DONE bit
    integerResult = (ADRESH << 8) | ADRESL;      //Combine high and low bits into single 10-bit integer
    // The precise formula for getting the measured voltage in millivolts is:
    // (integerResult/1023.0)*(POSVREF-NEGVREF)*1000
    // However, these constants are approximately 2, so we can speed up
    // this function by keeping everything as integers.
    return integerResult*2;
}