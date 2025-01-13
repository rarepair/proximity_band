#include <xc.h>
#include "Rx_PD.h"
#include "main.h"

//Enables/disables the peak detector circuit. The peak detector sampling cap is
//first partially disacharged before enabling the comparator.
void PD_detect(char detect)
{
    if (detect == TRUE)
    {
        LATC |= 0b00000100;         //Set PD_CLEAR (RC2) high to discharge storage cap
        __delay_us(PD_CLEARTIME);
        LATC &= 0b11111011;         //Set PD_CLEAR (RC2) low to allow PD to charge storage cap
        LATC |= 0b00010000;         //Set PD_SHDN_L (RC4) high to enable comparator
    }
    else
        LATC &= 0b11101111;         //Set PD_SHDN_L (RC4) low to put comparator in reset
}