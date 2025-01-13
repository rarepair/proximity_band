#ifndef DAC_H
#define	DAC_H

#define DAC_NEGVREF_EXT 1
#define DAC_NEGVREF_VSS 0
#define DAC_POSVREF_FVRBUFOUT 2
#define DAC_POSVREF_EXT 1
#define DAC_POSVREF_VDD 0
#define DAC_OPA2BUF_EN 1
#define DAC_OPA2BUF_DIS 0

struct DAC 
{
    unsigned char DAC_posvrefsrc;
    unsigned char DAC_negvrefsrc;
    unsigned char DAC_opampbuf;
};

void DAC_init(struct DAC dev);
void DAC_output(char level);

#endif	/* DAC_H */

