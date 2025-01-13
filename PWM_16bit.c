#include <xc.h>
#include "PWM_16bit.h"
#include "main.h"

void PWM16_init(struct PWM16 dev)
{
    unsigned char TMR1_b = 0b00000000;
    
    switch (dev.TMR1_clksrc)
    {
        /*case TMR1_CLKSRC_LFINTOSC:
            TMR1_b |= 0b11000000;
            break;
        case TMR1_CLKSRC_EXTERN:
            TMR1_b |= 0b10000000;
            break;
        case TMR1_CLKSRC_FOSC:
            TMR1_b |= 0b01000000;
            break;*/
        case TMR1_CLKSRC_FOSC4:
            break;
        default:
            return;
    }
    
    switch (dev.TMR1_clkprescale)
    {
        /*case TMR1_CLKPRESCALE_8:
            TMR1_b |= 0b00110000;
            break;
        case TMR1_CLKPRESCALE_4:
            TMR1_b |= 0b00100000;
            break;
        case TMR1_CLKPRESCALE_2:
            TMR1_b |= 0b00010000;
            break;*/
        case TMR1_CLKPRESCALE_1:
            break;
        default:
            return;
    }

    if (dev.TMR1_lposc == TMR1_LPOSC_EN)
        TMR1_b |= 0b00001000;
    
    if (dev.TMR1_async == TMR1_ASYNC_SYNCDIS)
        TMR1_b |= 0b00000100;
    
    T1CON = TMR1_b;
    
    switch (dev.CCP1_mode)
    {
        /*case CCP1_MODE_GENSOFTINTERRUPT:
            CCP1CON = 0b00001010;
            CCP1_mode = 0b00001010;
            break;*/
        case CCP1_MODE_CLRONCOMPAREMATCH:
            CCP1CON = 0b00001001;
            CCP1_mode = 0b00001001;
            break;
        /*case CCP1_MODE_SETONCOMPAREMATCH:
            CCP1CON = 0b00001000;
            CCP1_mode = 0b00001000;
            break;*/
        default:
            return;
    }
}

//Enables all necessary modules for the 16-bit PWM to work. 
void PWM16_enable(char state)
{
    if (state == TRUE)
    {
        CCP1CON = CCP1_mode;            //Re-enable CCP1 so that it starts with the correct output value
        PIR1 &= 0b11111110;             //Clear the Timer1 overflow interrupt flag
        PIE1 |= 0b00000001;             //Enable Timer1 overflow interrupt
        T1CON |= 0b00000001;            //Enable Timer1
    }
    else
    {
        T1CON &= 0b11111110;            //Disable Timer1
        TMR1H = 0;                      //Clear TMR1 value in case it is not already 0
        TMR1L = 0;                      //
        CCP1CON = 0b00000000;           //Reset CCP1 module latch
        PIE1 &= 0b11111110;             //Disable Timer1 overflow interrupt
    }
}

//Set the values of the "compare" register in the CCP1 module to the duty cycle.
//Timer1 counts up to this value and then asserts an interrupt when they are equal.
void PWM16_setDuty(unsigned int duty)
{
    CCPR1H = duty >> 8;
    CCPR1L = duty;
}

