// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection Bits (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover Mode (Internal/External Switchover Mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = ALL        // Flash Memory Self-Write Protection (000h to 1FFFh write protected, no addresses may be modified by EECON control)
#pragma config PPS1WAY = OFF    // Peripheral Pin Select one-way control (The PPSLOCK bit can be set and cleared repeatedly by software)
#pragma config ZCDDIS = ON      // Zero-cross detect disable (Zero-cross detect circuit is disabled at POR)
#pragma config PLLEN = ON       // Phase Lock Loop enable (4x PLL is always enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = HI        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), high trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#define _XTAL_FREQ 32000000
#define OSC32MHZ 0xF0
#define OSC16MHZ 0x78
#define OSC8MHZ 0x70
#define OSC4MHZ 0x68
#define OSC2MHZ 0x60
#define OSC1MHZ 0x58
#define OSC500KHz 0x38
#define TRUE 1
#define FALSE 0
#define POSVREF 2.048          // Positive voltage reference for ADC (V)
#define NEGVREF 0.0            // Negative voltage reference for ADC (V)
#define MAX_VPD 1750           // Maximum detectable voltage by the peak detector (mV)
#define ADPTVESMPLEERR 10      // Acceptable error value between successive samples when detecting Rx signal strength (mV)
#define AMPCMCHRGTIME 10       // Time for amplifier common mode to settle after changing the gain (us)
#define D_MAX 0xffff           // Maximum duty cycle value for LED driver (counts out of 16bits)
#define D_MIN 0x10             // Minimum duty cycle value for LED driver (counts out of 16bits)
#define D_TABLE_SIZE 768       // Size of table that maps measured voltage to output duty cycle
#define A_MAX 750.0            // Equivalent maximum amplitude (on highest amplification) that receiver can detect
#define A_MIN 0.47             // Equivalent minimum amplitude (on lowest amplification) that receiver can detect
#define PREAMPGAIN 4.0         // Fixed gain of preamplifier that drives PGA (V/V)
#define MAX_DETSDDNZEROS 2     // Maximum number of zero samples to skip before adding to rolling average
#define PD_TRACKING_TIME 125   // Time for PD to track a maximum amplitude signal (us)

void initMicro(void);
void __interrupt() ISR();
unsigned int sampleRxAdaptive(void);
unsigned int sampleRx(void);
unsigned int RxDetect(void);
double getRxAmplitude(unsigned int value);
unsigned int getDutyFromAmplitude(double a_det);
void BuffAmp_enable(char state);
void signalFatalError(void);