#include <xc.h>
#include "Tx_Oscillator.h"
#include "main.h"

//Enable/disable the Tx oscillator source, which is fed into a pulse generation
//circuit to drive the Tx coil transistor. Another transistor in the coil network
//is use to quickly squelch the oscillations so that Rx measurements can be performed
//quickly after disabling the Tx osc without interference.
void TxOsc_enable(char state)
{
    if (state == FALSE)
    {
        LATA &= 0b11011111;             //Set Tx_ENABLE (RA5) low to turn off Tx oscillator
        LATC |= 0b00100000;             //Set TX_KILL (RC5) high to squelch Tx coil current
        __delay_us(30);                 //Wait for oscillations to stop before returning   
    }   
    else
    {
        LATC &= 0b11011111;             //Set TX_KILL (RC5) low to allow Tx coil current
        __delay_us(3);                  //Wait for TX_KILL to settle before starting osc
        LATA |= 0b00100000;             //Set Tx_ENABLE (RA5) pin high to turn on Tx oscillator
    }
}
