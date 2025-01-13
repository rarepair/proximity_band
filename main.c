#include <stdlib.h>
#include <xc.h>
#include <math.h>
#include "main.h"
#include "SPI.h"
#include "Tx_Oscillator.h"
#include "Rx_PGA.h"
#include "Rx_PD.h"
#include "RollingAverage.h"
#include "PWM_16bit.h"
#include "Timer2.h"
#include "ADC.h"
#include "DAC.h"

struct ADC ADCdev;
struct DAC DACdev;
struct SPI SPIdev_PGA;
struct PWM16 PWM16dev;
struct TIMER2 TMR2dev;
static volatile __bit acqGate;

// Color/serial number selection of calibration values
#define BLUE_001 1
//#define RED_002 1
//#define GREEN_003 1
int main(void)
{
    initMicro();
    PWM16_setDuty(D_MIN);
    PWM16_enable(FALSE);
    // Tune the Tx oscillator frequency for each unit
    #if defined(BLUE_001)
        DAC_output(0x5D);
    #elif defined(RED_002)
        DAC_output(0x76);
    #elif defined(GREEN_003)
        DAC_output(0x74);
    #else
        signalFatalError();
    #endif
    TxOsc_enable(FALSE);
    BuffAmp_enable(FALSE);
    ADC_enable(TRUE);
    RxPGA_init(SPIdev_PGA);
    PD_detect(FALSE);

    __delay_ms(300);                    //Allow for Vref_Buff to charge up to Vdd/2
    RxPGA_enable(TRUE);
    if (RxPGA_calVOS() != 0)            //Calibrate PGA input offset voltage
        signalFatalError();

    //-------------------Debug Sequences-------------------//
    /*/Rx Coil Tuning Sequence
    //This code keeps the receiver chain on in the highest gain setting and turns
    //everything else off.
    //Use a function generator to drive a 480KHz sinusoidal waveform into a
    //coil. Vary the frequency around 480KHz to find where the highest sensitivity
    //is reached by probing the output of the VGA. Use parallel resonator equations
    //to figure out how much capacitance to add/remove. Repeat until optimized.
    TxOsc_enable(FALSE);
    PD_detect(TRUE);
    BuffAmp_enable(TRUE);
    RxPGA_enable(TRUE);
    RxPGA_setGain(RXGAIN157);
    while(TRUE) { NOP(); };
    /*Rx Coil Tuning Sequence*/    
    /*/Tx Coil Tuning Sequence
    //This code turns on the Tx oscillator and turns off everything else.
    //Inject a DC signal from function generator into SET pin of Tx oscillator on
    //board and probe amplitude of coil drive waveform. Find frequency at which
    //output amplitude is maximized and use parallel resonator equations to calculate
    //how much capacitance to add/remove from node to optimize amplitude at 480KHz.
    TxOsc_enable(TRUE);
    PD_detect(FALSE);
    BuffAmp_enable(FALSE);
    RxPGA_enable(FALSE);
    while(TRUE) { NOP(); };
    /*Tx Coil Tuning Sequence*/

    unsigned int Vdet = 0;
    unsigned char suddenZeroCount = 0;
    unsigned int currDdet = 0;
    unsigned int Davg = 0;
    acqGate = FALSE;
    TMR2_enable(TRUE);
    while(TRUE)
    {
        if (acqGate == TRUE)
        {   
            TxOsc_enable(FALSE);
            PWM16_enable(FALSE);
            //Can only set duty cycle when PWM is off, or glitch may occur when
            //certain values are loaded (since lower and upper bytes are loaded
            //separately).
            PWM16_setDuty(Davg);
            RxPGA_enable(TRUE);
            BuffAmp_enable(TRUE);
            Vdet = RxDetect();
            BuffAmp_enable(FALSE);
            RxPGA_enable(FALSE);
            PWM16_enable(TRUE);
            TxOsc_enable(TRUE);
            
            //If we suddenly measure a zero, it might be because we are sampling
            //the partners signal at the same time that is sampling ours. Ignore
            //up to MAX_DETSDDNZEROS of these events before inserting them to  
            //the rolling average.
            currDdet = getDutyFromAmplitude(getRxAmplitude(Vdet));
            if (suddenZeroCount < MAX_DETSDDNZEROS && RA_getLastEntry() > (D_MIN + 0x10) && currDdet == D_MIN) {
                //If it's a sudden zero and we're below the limit, skip the
                //sample and increment the counter
                suddenZeroCount++;
            } else {
                //Otherwise, add the sample to the rolling average as normal
                RA_insert(currDdet);
                Davg = RA_getAvg();
                suddenZeroCount = 0;
            }            
            acqGate = FALSE;
        }
    };
    return EXIT_SUCCESS;
}

void initMicro(void)
{
    OSCCON = OSC32MHZ;                                //Set internal oscillator to 32MHz

    //Set up pin source selection registers
    RA0PPS = 0b00001100;                              //Set RA0 (LED_EN_PWM) as CCP1
    RC0PPS = 0b00010010;                              //Set RC4 (SPI_MOSI) as SDO
    RC1PPS = 0b00010000;                              //Set RC5 (SPI_SCLK) as SCK

    //Set up RAx pins as inputs/outputs and drive low
    PORTA = 0x00;                                 //Set all PORTA pins to drive low
    TRISAbits.TRISA0 = 0;                         //Set ICSPDAT/LED_EN_PWM pin as output
    TRISAbits.TRISA1 = 0;                         //Set ICSPCLK/BUFF_AMP_SHDN pin as output
    TRISAbits.TRISA2 = 0;                         //Set SPI_CS pin as output
    TRISAbits.TRISA4 = 1;                         //Set ADC_IN pin as input
    TRISAbits.TRISA5 = 0;                         //Set Tx_ENABLE pin as output
    
    //Set up RCx pins as inputs/outputs and drive low
    PORTC = 0x00;                                 //Set all PORTC pins to drive low
    TRISCbits.TRISC0 = 0;                         //Set SPI_MOSI pin as output
    TRISCbits.TRISC1 = 0;                         //Set SPI_SCLK pin as output
    TRISCbits.TRISC2 = 0;                         //Set PD_CLEAR pin as output
    TRISCbits.TRISC3 = 0;                         //Set DAC_OUT pin as output
    TRISCbits.TRISC4 = 0;                         //Set PD_SHDN_L pin as output
    TRISCbits.TRISC5 = 0;                         //Set TX_KILL pin as output

    //PD ADC set-up
    ANSELAbits.ANSA4 = 1;                         //Set up ADC_IN pin (RA4 = AN3 = pin 2) as analog
    FVRCON = 0b10001010;                          //Set FVR to output to DAC and ADC with 2X gain (2.048V) and enable FVR
    ADCdev.ADC_clksrc = ADC_CLKSRC_FOSC32;
    ADCdev.ADC_chansrc = ADC_INPUT_AN3;
    ADCdev.ADC_negvrefsrc = ADC_NEGVREF_VSS;
    ADCdev.ADC_posvrefsrc = ADC_POSVREF_FVR;
    ADCdev.ADC_resultfmt = ADC_RESULT_RIGHTJUST;
    ADC_init(ADCdev);
    
    //TxOSC DAC set-up
    DACdev.DAC_negvrefsrc = DAC_NEGVREF_VSS;
    DACdev.DAC_posvrefsrc = DAC_POSVREF_FVRBUFOUT;
    DACdev.DAC_opampbuf = DAC_OPA2BUF_EN;
    DAC_init(DACdev);
    
    //PGA SPI communication set-up
    SPIdev_PGA.clockMode = SPI_CLKSRC_FOSC4;
    SPIdev_PGA.dataSampleMode = SPI_SAMPLEMODE_MID;
    SPIdev_PGA.idleState = SPI_CLKIDLE_LOW;
    SPIdev_PGA.transmitMode = SPI_TXCLKEDGE_ACTIVETOIDLE;
    SPIdev_PGA.endianness = SPI_LSBFIRST;
    SPIdev_PGA.chipSelectPort = (unsigned char *)&LATA;
    SPIdev_PGA.chipSelectBit = 2;
    SPI_init(SPIdev_PGA);
    SPI_enable(TRUE);
    
    //Initialize the rolling average for LED duty cycle values
    RA_init();
    
    //16-bit PWM set-up
    PWM16dev.TMR1_clksrc = TMR1_CLKSRC_FOSC4;
    PWM16dev.TMR1_clkprescale = TMR1_CLKPRESCALE_1;
    PWM16dev.TMR1_lposc = TMR1_LPOSC_DIS;
    PWM16dev.TMR1_async = TMR1_ASYNC_SYNCEN;
    PWM16dev.CCP1_mode = CCP1_MODE_CLRONCOMPAREMATCH;
    PWM16_init(PWM16dev);
    
    //Timer2 signal acquisition timer set-up
    TMR2dev.TMR2_prescaler = TMR2_PRESCALER_64;
    TMR2dev.TMR2_postscaler = TMR2_POSTSCALER_16;
    //Choose a different sample rate for each unit. This ensures the "blanking
    //period", which is when two or more units are simultaneously sampling Rx
    //power (and hence both will suddenly measure zero), is sufficiently short.
    #if defined(BLUE_001)
        TMR2dev.TMR2_maxcount = 100;                //Trx=12.800ms, Frx=78.125Hz
    #elif defined(RED_002)
        TMR2dev.TMR2_maxcount = 120;                //Trx=15.360ms, Frx=65.104Hz
    #elif defined(GREEN_003)
        TMR2dev.TMR2_maxcount = 140;                //Trx=17.920ms, Frx=55.804Hz
    #else
        signalFatalError();
    #endif
    TMR2_init(TMR2dev);
    
    //Enable interrupts globally and enable peripheral interrupts
    INTCON |= 0b11000000;
}

void __interrupt() ISR()
{
    if ((PIR1 & 0b00000001) == TRUE)    //Check if Timer1 overflow is interrupt source
    {
        CCP1CON = 0b00000000;           //Reset CCP1 module latch before giving RA2 pin control back to it
        CCP1CON = CCP1_mode;            //Required or it will drive the pin with the previous latch value
        PIR1 &= 0b11111110;             //Clear the Timer1 overflow interrupt flag
    }
    else                                //TMR2 to PR2 match is interrupt source
    {
        acqGate = TRUE;                 //Permit main loop to measure signal amplitude
        PIR1 &= 0b11111101;             //Clear the Timer1 overflow interrupt flag
    }
}

//Measure the Rx signal strength by tracking the peak output of the PGA with
//the peak detector and sampling with the ADC.
unsigned int sampleRx(void)
{
    PD_detect(TRUE);
    __delay_us(PD_TRACKING_TIME);
    unsigned int Vadc = ADC_convert();
    PD_detect(FALSE);
    return Vadc;
}

//Measures the Rx signal strength by tracking the peak output of the PGA with
//the peak detector and sampling with the ADC. It uses an adaptive algorithm
//that continuously samples until the value is changing less than 10mV between
//consecutive samples. 
unsigned int sampleRxAdaptive(void)
{
    unsigned int prev_Vadc = 0;
    unsigned int curr_Vadc = 0;
    char loopcount = 0;
    PD_detect(TRUE);
    do {
        prev_Vadc = curr_Vadc;
        curr_Vadc = ADC_convert();
        loopcount += 1;
    } while ((abs(curr_Vadc - prev_Vadc) > ADPTVESMPLEERR) && (loopcount < 15));
    //At this point, the signal at the ADC input is changing slowly. Wait
    //another 10us and take the final sample
    __delay_us(10);
    curr_Vadc = ADC_convert();
    PD_detect(FALSE);
    return curr_Vadc;
}

//Determines the Rx signal amplitude by measuring amplified output voltage of 
//receiver chain at increasing gain ranges.
//This function must run as fast as possible, since the LEDs and transmitter must
//be off in order to not overwhelm the receiver with noise. It must execute quickly
//enough that no flicker in the LEDs is visible to the eye.
//The PD must be turned off between gain changes because the PGA creates large glitches
//when switching to high gain settings that get tracked by the PD.
//Returns the ADC measured amplitude with the PGA gain encoded in the value for 
//maximum computational efficiency.
unsigned int RxDetect(void)
{
    unsigned int Vadc = 0;
    
    RxPGA_setGain(RXGAIN1);
    Vadc = sampleRxAdaptive();
    if (Vadc*RXGAIN10 >= MAX_VPD)
        return Vadc;
    
    RxPGA_setGain(RXGAIN10);
    __delay_us(AMPCMCHRGTIME);
    Vadc = sampleRxAdaptive();
    if (Vadc*(RXGAIN40/RXGAIN10) >= MAX_VPD)
        return Vadc + 2000;
    
    RxPGA_setGain(RXGAIN40);
    __delay_us(AMPCMCHRGTIME);
    Vadc = sampleRxAdaptive();
    if (Vadc*(RXGAIN157/RXGAIN40) >= MAX_VPD)
        return Vadc + 4000;        
    
    RxPGA_setGain(RXGAIN157);
    __delay_us(AMPCMCHRGTIME);
    Vadc = sampleRxAdaptive();
    return Vadc + 6000;
}

//Takes an encoded input value consisting of a voltage + encoded gain and calculates
//the actual Rx amplitude detected by receiver.
//This function is slow due to floating point math and must be run after signal
//detection has completed, while the LEDs and transmitter are on.
double getRxAmplitude(unsigned int value)
{
    double gain = RXGAIN1;
    
    if (value > 6000)
    {
        gain = (double)RXGAIN157;
        if (value <= (6000.0 + A_MIN*PREAMPGAIN*gain))
            return 0;
        else
            value -= 6000;
    }
    else if (value > 4000)
    {
        gain = (double)RXGAIN40;
        value -= 4000;
    }
    else if (value > 2000)
    {
        gain = (double)RXGAIN10;
        value -= 2000;
    }
    
    return ((double)value/(PREAMPGAIN*gain));
}

//Returns the duty cycle value that corresponds to the detected Rx signal
//amplitude. The values in the LUT are computed with a Matlab script that
//compensates for sources of nonlinearity: 
//1. The human eye doesn't perceive brightness linearly
//2. The magnetic field strength of a coil drops off nonlinearly with distance
unsigned int getDutyFromAmplitude(double a_det)
{
    static const unsigned int D[D_TABLE_SIZE] = {
		0x10, 0x12, 0x14, 0x16, 0x17, 0x19, 0x1b, 0x1d,
		0x1f, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2b, 0x2e,
		0x30, 0x32, 0x34, 0x36, 0x38, 0x3b, 0x3d, 0x3f,
		0x41, 0x44, 0x46, 0x48, 0x4b, 0x4d, 0x4f, 0x52,
		0x54, 0x57, 0x59, 0x5c, 0x5e, 0x61, 0x63, 0x66,
		0x69, 0x6b, 0x6e, 0x71, 0x73, 0x76, 0x79, 0x7c,
		0x7f, 0x81, 0x84, 0x87, 0x8a, 0x8d, 0x90, 0x93,
		0x96, 0x99, 0x9c, 0x9f, 0xa3, 0xa6, 0xa9, 0xac,
		0xaf, 0xb3, 0xb6, 0xb9, 0xbd, 0xc0, 0xc4, 0xc7,
		0xcb, 0xce, 0xd2, 0xd5, 0xd9, 0xdd, 0xe0, 0xe4,
		0xe8, 0xec, 0xef, 0xf3, 0xf7, 0xfb, 0xff, 0x103,
		0x107, 0x10b, 0x10f, 0x113, 0x117, 0x11c, 0x120, 0x124,
		0x128, 0x12d, 0x131, 0x136, 0x13a, 0x13f, 0x143, 0x148,
		0x14c, 0x151, 0x156, 0x15a, 0x15f, 0x164, 0x169, 0x16e,
		0x173, 0x178, 0x17d, 0x182, 0x187, 0x18c, 0x191, 0x196,
		0x19c, 0x1a1, 0x1a6, 0x1ac, 0x1b1, 0x1b7, 0x1bc, 0x1c2,
		0x1c7, 0x1cd, 0x1d3, 0x1d9, 0x1de, 0x1e4, 0x1ea, 0x1f0,
		0x1f6, 0x1fc, 0x202, 0x209, 0x20f, 0x215, 0x21b, 0x222,
		0x228, 0x22f, 0x235, 0x23c, 0x242, 0x249, 0x250, 0x257,
		0x25d, 0x264, 0x26c, 0x273, 0x27a, 0x282, 0x289, 0x291,
		0x299, 0x2a1, 0x2a9, 0x2b1, 0x2b9, 0x2c2, 0x2ca, 0x2d3,
		0x2dc, 0x2e5, 0x2ee, 0x2f7, 0x301, 0x30a, 0x314, 0x31e,
		0x328, 0x332, 0x33d, 0x347, 0x352, 0x35d, 0x368, 0x373,
		0x37f, 0x38b, 0x396, 0x3a2, 0x3af, 0x3bb, 0x3c8, 0x3d5,
		0x3e2, 0x3ef, 0x3fc, 0x40a, 0x418, 0x426, 0x434, 0x443,
		0x452, 0x461, 0x470, 0x480, 0x490, 0x4a0, 0x4b0, 0x4c1,
		0x4d2, 0x4e3, 0x4f4, 0x506, 0x518, 0x52a, 0x53d, 0x550,
		0x563, 0x576, 0x58a, 0x59e, 0x5b3, 0x5c7, 0x5dd, 0x5f2,
		0x608, 0x61e, 0x635, 0x64b, 0x663, 0x67a, 0x692, 0x6ab,
		0x6c4, 0x6dd, 0x6f6, 0x710, 0x72b, 0x746, 0x761, 0x77c,
		0x799, 0x7b5, 0x7d2, 0x7f0, 0x80e, 0x82c, 0x84b, 0x86a,
		0x88a, 0x8ab, 0x8cb, 0x8ed, 0x90f, 0x931, 0x954, 0x978,
		0x99c, 0x9c0, 0x9e6, 0xa0b, 0xa32, 0xa59, 0xa80, 0xaa8,
		0xad1, 0xafb, 0xb25, 0xb4f, 0xb7b, 0xba6, 0xbd3, 0xc00,
		0xc2e, 0xc5d, 0xc8c, 0xcbc, 0xced, 0xd1e, 0xd51, 0xd84,
		0xdb7, 0xdec, 0xe21, 0xe57, 0xe8e, 0xec5, 0xefd, 0xf37,
		0xf71, 0xfab, 0xfe7, 0x1023, 0x1061, 0x109f, 0x10de, 0x111e,
		0x115f, 0x11a0, 0x11e3, 0x1227, 0x126b, 0x12b0, 0x12f7, 0x133e,
		0x1386, 0x13d0, 0x141a, 0x1465, 0x14b1, 0x14fe, 0x154d, 0x159c,
		0x15ec, 0x163d, 0x1690, 0x16e3, 0x1738, 0x178d, 0x17e4, 0x183b,
		0x1894, 0x18ee, 0x1949, 0x19a5, 0x1a03, 0x1a61, 0x1ac1, 0x1b21,
		0x1b83, 0x1be6, 0x1c4b, 0x1cb0, 0x1d17, 0x1d7f, 0x1de8, 0x1e52,
		0x1ebd, 0x1f2a, 0x1f98, 0x2007, 0x2077, 0x20e9, 0x215c, 0x21d0,
		0x2245, 0x22bc, 0x2334, 0x23ad, 0x2427, 0x24a3, 0x2520, 0x259e,
		0x261e, 0x269f, 0x2721, 0x27a4, 0x2829, 0x28af, 0x2937, 0x29bf,
		0x2a49, 0x2ad5, 0x2b61, 0x2bef, 0x2c7f, 0x2d0f, 0x2da1, 0x2e34,
		0x2ec9, 0x2f5f, 0x2ff6, 0x308f, 0x3128, 0x31c3, 0x3260, 0x32fe,
		0x339d, 0x343d, 0x34df, 0x3581, 0x3626, 0x36cb, 0x3772, 0x381a,
		0x38c3, 0x396e, 0x3a19, 0x3ac6, 0x3b75, 0x3c24, 0x3cd5, 0x3d87,
		0x3e3a, 0x3eef, 0x3fa4, 0x405b, 0x4113, 0x41cc, 0x4286, 0x4342,
		0x43fe, 0x44bc, 0x457b, 0x463b, 0x46fc, 0x47be, 0x4881, 0x4946,
		0x4a0b, 0x4ad1, 0x4b99, 0x4c61, 0x4d2b, 0x4df5, 0x4ec0, 0x4f8d,
		0x505a, 0x5128, 0x51f7, 0x52c7, 0x5398, 0x546a, 0x553d, 0x5610,
		0x56e4, 0x57b9, 0x588f, 0x5966, 0x5a3d, 0x5b15, 0x5bee, 0x5cc8,
		0x5da2, 0x5e7d, 0x5f58, 0x6034, 0x6111, 0x61ee, 0x62cc, 0x63ab,
		0x6489, 0x6569, 0x6649, 0x6729, 0x680a, 0x68eb, 0x69cd, 0x6aaf,
		0x6b92, 0x6c75, 0x6d58, 0x6e3b, 0x6f1f, 0x7003, 0x70e7, 0x71cc,
		0x72b0, 0x7395, 0x747a, 0x7560, 0x7645, 0x772a, 0x7810, 0x78f5,
		0x79db, 0x7ac1, 0x7ba6, 0x7c8c, 0x7d72, 0x7e57, 0x7f3d, 0x8022,
		0x8107, 0x81ec, 0x82d1, 0x83b6, 0x849b, 0x857f, 0x8663, 0x8747,
		0x882b, 0x890e, 0x89f1, 0x8ad4, 0x8bb6, 0x8c98, 0x8d7a, 0x8e5b,
		0x8f3c, 0x901c, 0x90fc, 0x91db, 0x92ba, 0x9398, 0x9476, 0x9554,
		0x9630, 0x970c, 0x97e8, 0x98c3, 0x999d, 0x9a77, 0x9b50, 0x9c29,
		0x9d00, 0x9dd7, 0x9eae, 0x9f83, 0xa058, 0xa12c, 0xa200, 0xa2d2,
		0xa3a4, 0xa475, 0xa545, 0xa614, 0xa6e3, 0xa7b0, 0xa87d, 0xa949,
		0xaa14, 0xaade, 0xaba7, 0xac6f, 0xad37, 0xadfd, 0xaec3, 0xaf87,
		0xb04b, 0xb10d, 0xb1cf, 0xb28f, 0xb34f, 0xb40d, 0xb4cb, 0xb587,
		0xb643, 0xb6fd, 0xb7b7, 0xb86f, 0xb926, 0xb9dd, 0xba92, 0xbb46,
		0xbbf9, 0xbcab, 0xbd5c, 0xbe0c, 0xbeba, 0xbf68, 0xc014, 0xc0c0,
		0xc16a, 0xc213, 0xc2bb, 0xc362, 0xc408, 0xc4ad, 0xc550, 0xc5f3,
		0xc694, 0xc734, 0xc7d3, 0xc871, 0xc90e, 0xc9aa, 0xca44, 0xcade,
		0xcb76, 0xcc0d, 0xcca3, 0xcd38, 0xcdcc, 0xce5e, 0xcef0, 0xcf80,
		0xd00f, 0xd09e, 0xd12b, 0xd1b6, 0xd241, 0xd2cb, 0xd353, 0xd3db,
		0xd461, 0xd4e6, 0xd56a, 0xd5ed, 0xd66f, 0xd6f0, 0xd770, 0xd7ee,
		0xd86c, 0xd8e8, 0xd964, 0xd9de, 0xda57, 0xdacf, 0xdb46, 0xdbbc,
		0xdc31, 0xdca5, 0xdd18, 0xdd8a, 0xddfb, 0xde6b, 0xded9, 0xdf47,
		0xdfb4, 0xe01f, 0xe08a, 0xe0f4, 0xe15d, 0xe1c4, 0xe22b, 0xe291,
		0xe2f5, 0xe359, 0xe3bc, 0xe41e, 0xe47f, 0xe4df, 0xe53e, 0xe59c,
		0xe5f9, 0xe656, 0xe6b1, 0xe70b, 0xe765, 0xe7be, 0xe815, 0xe86c,
		0xe8c2, 0xe917, 0xe96b, 0xe9bf, 0xea11, 0xea63, 0xeab4, 0xeb04,
		0xeb53, 0xeba1, 0xebef, 0xec3c, 0xec87, 0xecd3, 0xed1d, 0xed66,
		0xedaf, 0xedf7, 0xee3e, 0xee85, 0xeeca, 0xef0f, 0xef53, 0xef97,
		0xefda, 0xf01c, 0xf05d, 0xf09d, 0xf0dd, 0xf11c, 0xf15b, 0xf199,
		0xf1d6, 0xf212, 0xf24e, 0xf289, 0xf2c3, 0xf2fd, 0xf336, 0xf36e,
		0xf3a6, 0xf3dd, 0xf414, 0xf44a, 0xf47f, 0xf4b4, 0xf4e8, 0xf51c,
		0xf54f, 0xf581, 0xf5b3, 0xf5e4, 0xf615, 0xf645, 0xf674, 0xf6a3,
		0xf6d2, 0xf700, 0xf72d, 0xf75a, 0xf786, 0xf7b2, 0xf7dd, 0xf808,
		0xf832, 0xf85c, 0xf885, 0xf8ae, 0xf8d7, 0xf8fe, 0xf926, 0xf94d,
		0xf973, 0xf999, 0xf9bf, 0xf9e4, 0xfa08, 0xfa2c, 0xfa50, 0xfa74,
		0xfa96, 0xfab9, 0xfadb, 0xfafd, 0xfb1e, 0xfb3f, 0xfb5f, 0xfb7f,
		0xfb9f, 0xfbbe, 0xfbdd, 0xfbfc, 0xfc1a, 0xfc38, 0xfc55, 0xfc72,
		0xfc8f, 0xfcab, 0xfcc7, 0xfce3, 0xfcfe, 0xfd19, 0xfd34, 0xfd4e,
		0xfd68, 0xfd82, 0xfd9b, 0xfdb4, 0xfdcd, 0xfde5, 0xfdfd, 0xfe15,
		0xfe2d, 0xfe44, 0xfe5b, 0xfe71, 0xfe88, 0xfe9e, 0xfeb4, 0xfec9,
		0xfede, 0xfef3, 0xff08, 0xff1d, 0xff31, 0xff45, 0xff59, 0xff6c,
		0xff7f, 0xff92, 0xffa5, 0xffb7, 0xffca, 0xffdc, 0xffed, 0xffff,
    };

    if (a_det < A_MIN)
        return D[0];
    else if (a_det > A_MAX)
        return D[D_TABLE_SIZE - 1];
    
    int eta = (int)round((D_TABLE_SIZE-1)*log10(a_det/A_MIN)/log10(A_MAX/A_MIN));
    return D[eta];
}

//Turns of the pre-amplifier stage on or off.
void BuffAmp_enable(char state)
{
    if (state == FALSE)
        LATA |= 0b00000010;    //Set BUFF_AMP_SHDN high to power down amp
    else
        LATA &= 0b11111101;    //Set BUFF_AMP_SHDN low to turn on amp
}

//If a fatal error condition is reached, this function can be called to enter an
//infinite loop that blinks the LEDs on and off to alert the user.
void signalFatalError()
{
    PWM16_setDuty(0x2000);
    while(TRUE)
    {
        PWM16_enable(TRUE);
        __delay_ms(300);
        PWM16_enable(FALSE);
        __delay_ms(300);
    }
}