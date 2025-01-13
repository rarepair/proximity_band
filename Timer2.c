#include <xc.h>
#include "Timer2.h"
#include "main.h"

void TMR2_init(struct TIMER2 dev)
{
    unsigned char TMR2_b = 0b00000000;
    
    switch (dev.TMR2_prescaler)
    {
        case TMR2_PRESCALER_64:
            TMR2_b |= 0b00000011;
            break;
        /*case TMR2_PRESCALER_16:
            TMR2_b |= 0b00000010;
            break;
        case TMR2_PRESCALER_4:
            TMR2_b |= 0b00000001;
            break;
        case TMR2_PRESCALER_1:
            break;*/
        default:
            return;
    }
    
    switch (dev.TMR2_postscaler)
    {
        case TMR2_POSTSCALER_16:
            TMR2_b |= 0b01111000;
            break;
        /*case TMR2_POSTSCALER_15:
            TMR2_b |= 0b0111000;
            break;
        case TMR2_POSTSCALER_14:
            TMR2_b |= 0b01101000;
            break;
        case TMR2_POSTSCALER_13:
            TMR2_b |= 0b01100000;
            break;
        case TMR2_POSTSCALER_12:
            TMR2_b |= 0b01011000;
            break;
        case TMR2_POSTSCALER_11:
            TMR2_b |= 0b01010000;
            break;
        case TMR2_POSTSCALER_10:
            TMR2_b |= 0b01001000;
            break;
        case TMR2_POSTSCALER_9:
            TMR2_b |= 0b01000000;
            break;
        case TMR2_POSTSCALER_8:
            TMR2_b |= 0b00111000;
            break;
        case TMR2_POSTSCALER_7:
            TMR2_b |= 0b00110000;
            break;
        case TMR2_POSTSCALER_6:
            TMR2_b |= 0b00101000;
            break;
        case TMR2_POSTSCALER_5:
            TMR2_b |= 0b00100000;
            break;
        case TMR2_POSTSCALER_4:
            TMR2_b |= 0b00011000;
            break;
        case TMR2_POSTSCALER_3:
            TMR2_b |= 0b00010000;
            break;
        case TMR2_POSTSCALER_2:
            TMR2_b |= 0b00001000;
            break;
        case TMR2_POSTSCALER_1:
            break;*/
        default:
            return;
    }
    
    T2CON = TMR2_b;
    PR2 = dev.TMR2_maxcount;
}

void TMR2_enable(char state)
{
    if (state == TRUE)
    {
        TMR2 = 0;                       //Clear TMR2 value in case it is not already 0
        PIR1 &= 0b11111101;             //Clear peripheral interrupt 1 register
        PIE1 |= 0b00000010;             //Enable TMR2 to PR2 match interrupt
        T2CON |= 0b00000100;            //Enable Timer2
    }
    else
    {
        T2CON &= 0b11111101;            //Disable Timer2
        PIE1 &= 0b11111101;             //Disable TMR2 to PR2 match interrupt
    }
}