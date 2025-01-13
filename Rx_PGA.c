#include <xc.h>
#include "SPI.h"
#include "main.h"
#include "Rx_PGA.h"

static unsigned char RxPGAStatus;
static struct SPI SPIconf;

void RxPGA_init(struct SPI dev)
{
    RxPGAStatus = 0x0;
    SPIconf = dev;
}

void RxPGA_enable(char state)
{
    unsigned char ampStatus = RxPGAStatus;

    if (state == FALSE)
        ampStatus |= 0b10000000;
    else
        ampStatus = 0b00100001;         //Set gain=1, SHDN=0, MEAS=0
    
    SPI_writeByte(ampStatus, SPIconf);
    RxPGAStatus = ampStatus;
}

void RxPGA_setGain(unsigned char gain)
{
    //Copy current amp register value for shutdown and measure
    unsigned char ampStatus = RxPGAStatus & 0b11000000u;
   
    switch (gain)
    {
        /*case RXGAIN025:
            ampStatus |= 0b00010011;
            break;*/
        case RXGAIN1:
            ampStatus |= 0b00000001;
            break;
        case RXGAIN10:
            ampStatus |= 0b00000011;
            break;
        /*
        case RXGAIN20:
            ampStatus |= 0b00000101;
            break;
        case RXGAIN30:
            ampStatus |= 0b00000111;
            break;
        */
        case RXGAIN40:
            ampStatus |= 0b00001001;
            break;
        /*case RXGAIN60:
            ampStatus |= 0b00001011;
            break;*/
        /*case RXGAIN80:
            ampStatus |= 0b00001101;
            break;*/
        /*case RXGAIN120:
            ampStatus |= 0b00001111;
            break;*/
        case RXGAIN157:
            ampStatus |= 0b00010001;
            break;
        default:
            return;
    }
    
    SPI_writeByte(ampStatus, SPIconf);
    RxPGAStatus = ampStatus;
}

static void RxPGA_enableMeasureMode(char state)
{
    unsigned char ampStatus = RxPGAStatus;
    
    if (state == FALSE)
        ampStatus &= 0b10111111;
    else
        ampStatus |= 0b01000000;
    
    SPI_writeByte(ampStatus, SPIconf);
    RxPGAStatus = ampStatus;
}

static void RxPGA_setTrimVOS(unsigned char trimCode)
{
    //We don't need to save/restore ampStatus here since writes to the
    //trim register don't affect the gain register
    
    //Lower byte indicates trim code (between 0x0 and 0xF)
    //Higher byte indicates positive (0x0) or negative (0x1) trim direction
    if (trimCode > 0x1F)
        trimCode = 0x1F;

    //Construct trim register byte so that shutdown and measure mode are disabled
    trimCode <<= 1;
    trimCode &= 0b00111110;

    SPI_writeByte(trimCode, SPIconf);
}

//Performs an automatic calibration of the VGA input offset voltage. It does this
//by placing the VGA into "measure mode", where the inputs are shorted together,
//and sets the gain to maximum. It then steps through the possible offset calibration
//codes and measures the output of the amplifier with the peak detector and microcontroller
//ADC. When it finds the value that causes the VGA output to exceed VOSCALLIMIT,
//it chooses the previous offset code and returns.
//Note that strong external interference (i.e. a close by partner wristband) can
//cause this measurement process to select inaccurate results, so this must be
//performed when the wristband is far from external interferences.
unsigned char RxPGA_calVOS(void)
{
    unsigned char ampStatus = RxPGAStatus;
    unsigned char trimCode;
    unsigned int Vadc = 0;
    static __bit calDone = FALSE;
    
    RxPGA_enableMeasureMode(TRUE);
    RxPGA_setGain(RXGAIN157);
    
    LATC |= 0b00000100;         //Set PD_CLEAR (RC2) high to discharge storage cap
    __delay_us(1000);
    LATC &= 0b11111011;         //Set PD_CLEAR (RC2) low to allow PD to charge storage cap
    for (trimCode = 0x1F; trimCode >= 0x10; trimCode--) //Do negative trim values first
    {
        RxPGA_setTrimVOS(trimCode);
        Vadc = sampleRx();
        if (Vadc > VOSCALLIMIT)
        {
            calDone = TRUE;
            break;
        }
    }
    if (calDone != TRUE)
    {
        for (trimCode = 0x1; trimCode <= 0xF; trimCode++)  //Do positive trim values
        {
            RxPGA_setTrimVOS(trimCode);
            Vadc = sampleRx();
            if (Vadc > VOSCALLIMIT)
            {
                calDone = TRUE;
                break;
            }
        }
    }
    if (calDone != TRUE)    //Cal code could not be found
        return 1;
    if (trimCode >= 0x10)   //Values larger than 0x1F will be corrected in RxPGA_setTrimVOS()
        trimCode += 1;
    else if (trimCode <= 0xF)
        trimCode -= 1;

    RxPGA_setTrimVOS(trimCode);
    RxPGA_enableMeasureMode(FALSE);
    SPI_writeByte(ampStatus, SPIconf);
    return 0;
}