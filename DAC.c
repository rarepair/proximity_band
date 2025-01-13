#include <xc.h>
#include "DAC.h"
#include "main.h"

void DAC_init(struct DAC dev)
{
    unsigned char DAC1CON0_b = 0b00000000;
    
    switch (dev.DAC_posvrefsrc)
    {
        case DAC_POSVREF_FVRBUFOUT:
            DAC1CON0_b |= 0b00001000;
            break;
        /*case DAC_POSVREF_EXT:
            DAC1CON0_b |= 0b00000100;
            break;
        case DAC_POSVREF_VDD:
            break;*/
        default:
            return;
    }
    
    switch (dev.DAC_negvrefsrc)
    {
        /*case DAC_NEGVREF_EXT:
            DAC1CON0_b |= 0b00000001;
            break;*/
        case DAC_NEGVREF_VSS:
            break;
        default:
            return;
    }

    switch (dev.DAC_opampbuf)
    {
        case DAC_OPA2BUF_EN:
            OPA2CON = 0b11010010;
            break;
        /*case DAC_OPA2BUF_DIS:
            OPA2CON = 0b00000000;
            break;*/
        default:
            return;
    }
    DAC1CON0_b |= 0b10000000;             //Enable DAC here since we never need to disable
    DAC1CON0 = DAC1CON0_b;
}

void DAC_output(char level)
{
    DAC1CON1 = level;
}