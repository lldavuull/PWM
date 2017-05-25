
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded.  


extern void ADC_init(void);

extern void ADC_loop(void);

extern void ADC_interrupt(void);


/** ADC_Data Receive ADC_Data Buffer  */
volatile char ADC_Data;
/** ADC_FLAGS */
typedef struct {
    // Recieve Flags
    /**  Indicates new data was recieved */
    unsigned int New : 1;
    /** ADC_FLAGS Used to pass information from the ISR */
} ADC_FLAGS;
/** ADC_Flags Set in the ISR to indicate to helper functions etc */
volatile ADC_FLAGS ADC_Flags;