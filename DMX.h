/* 
 * File: 
 * Author: David
 * Comments: DMX
 * Revision history: 
 */



/**
*  Initialises the DMX512A Library
*  Please add near the beginning of you main() function before your main loop
*/
extern void DMX_init(void);

/**
* This Loop all the DMX512 related Interrupts.
* Please add this to the while(1) of your main() function.
*/
extern void DMX_loop(void);

/**
* This processes all the DMX512 related Interrupts.
* Please add this to your interrupt function.
*/
extern void DMX_interrupt(void);



/** Select the size of the receive buffer.
    This is the number of DMX512 Channels you will recieve
    For example a RGBW fixture may use 4 channels where a scanner may need 10 */
#define RX_BUFFER_SIZE 4

/** Selects the ms between breaks that sets the timeout flag.
    This is usually just over 1 second and is common for DMX Fixtures
    to go into a blackout or default mode if this occurs (i.e. signal lost) */
#define DMX_RX_TIMEOUT_MS 1200

