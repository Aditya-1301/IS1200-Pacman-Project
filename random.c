#include "pic32mx.h"  /* Declarations of system-specific addresses etc */


unsigned int seed() {
    int n = 8;
    unsigned int seed = n;
    for (; n > 0; n--) {
        /* Start sampling, wait until conversion is done */
        AD1CON1 |= (0x2);
        while (!(AD1CON1 & (0x2)))
            seed ^= ADC1BUF0 + TMR2;
        while (!(AD1CON1 & 0x1))
            seed ^= ADC1BUF0 + TMR2;

        /* Get the analog value */
        seed ^= ADC1BUF0 + TMR2;
    }
    return seed;
}