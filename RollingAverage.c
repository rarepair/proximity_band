#include <xc.h>
#include <math.h>
#include "RollingAverage.h"
#include "main.h"

static unsigned int window[RA_LEN];
static unsigned int avg;

//Initializes an array of integers for storing a rolling average.
void RA_init(void)
{
    unsigned char i;
    for (i=0; i<RA_LEN; i++)
        window[i-1] = 0u;
    avg = 0.0;
}

//Inserts a value into the moving average array (index 0 is the newest) and then
//computes the new average.
void RA_insert(unsigned int value)
{   
    //Shift array values and add new value to index 0
    char i;
    for (i=RA_LEN-1; i!=0; i--)
    {
        window[i] = window[i - 1u]; //the 'u' means unsigned; compiler complains otherwise
    }
    window[0] = value;

    //Sum up all elements and calculate the average
    double sum = 0.0;
    for (i=0; i<RA_LEN; i++)
    {
        sum += (double)window[i];
    }
    avg = (unsigned int)round(sum/RA_LEN);
}

unsigned int RA_getAvg(void)
{
    return avg;
}

unsigned int RA_getLastEntry(void)
{
    return window[0];
}