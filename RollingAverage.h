#ifndef ROLLINGAVERAGE_H
#define	ROLLINGAVERAGE_H

#define RA_LEN 10

void RA_init(void);
void RA_insert(unsigned int value);
unsigned int RA_getAvg(void);
unsigned int RA_getLastEntry(void);

#endif	/* ROLLINGAVERAGE_H */

