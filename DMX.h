/* 
 * File: 
 * Author: David
 * Comments: DMX
 * Revision history: 
 */
#include <stdint.h>

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
#define DMX_RX_TIMEOUT_MS 2400  //1200ms


/** RGBW Receive Data Buffer  */
volatile unsigned char RGBW[RX_BUFFER_SIZE];

/** RxData Receive Data Buffer  */
volatile unsigned char RxData[RX_BUFFER_SIZE];
/** RxChannel Base Address / Channel to start reading from */
int DMX_Address;
/** RxStart The Start Code for a valid data stream */
const char DMX_StartCode = 0;
/** AddrCount Address counter used in the interrupt routine */
volatile int RxAddrCount = 0;
/** *RxDataPtr Pointer to the receive data buffer. Used in interrupt */
volatile char *RxDataPtr;
/** RxState Current state in the receive state machine */
volatile char RxState = 0;
/** RxTimer Counts 0.5ms since last overflow - used for 1 second timeout */
volatile int RxTimer = 0;

//PWM SMOOTH//
/** DMXperiod Counts 500us since DMX period, period records when RX recieve DMX_StartCode*/
char DMXPeriod = 100;
/** DMXperiodConst incerease per 500us, Return to zero when RX recieve DMX_StartCode */
volatile char DMXPeriodConst =0;
/** DMXPeriodStep will change PWM at 500us Timer per DMXPeriodStep period */
char DMXPeriodStep[4] =1; //0 is unchange , 1 is always change per 0.5ms Timer up, 2 is 1ms, 3 is 1.5ms, ....
/** DMXPeriodStepconst decerease per 500us, when DMXPeriodStepconst decrease to zero, DMXPeriodStepconst will set to DMXPeriodStep and PWM will be changed */
char DMXPeriodStepConst[4] =0;



typedef struct {
    // Update PWM per 500us
    /**  00 =unchange  01 = Increment   10=Decrement */
        unsigned int X: 2;
} DMX_PERIODSIGN;
/** SMOOTH_Pwm Set in the ISR to indicate to helper functions etc */
volatile DMX_PERIODSIGN DMXPeriodSign[4]; /**  00 =unchange  01 = Increment   10=Decrement */
//PWM SMOOTH END//


enum {
    RX_WAIT_FOR_BREAK,
    WAIT_FOR_START,
    RX_DMX_READ_DATA,
    RX_RDM_READ_SubStartCode,
    RX_RDM_READ_DATA,
    RX_RDM_PD,
};

/** DMX_FLAGS */
typedef struct {
    // Recieve Flags
    /**  Indicates new data was recieved */
    unsigned int RxNew : 1;
    /**  Indicate valid break detected */
    unsigned int RxBreak : 1;
    /**  Indicates valid start is detected so store data */
    unsigned int RxStart : 1;
    /** Indicates 1 second timout occured */
    unsigned int RxTimeout : 1;
    // Transmit Flags
    /** Indicated transmission is in progress */
    unsigned int TxRunning : 1;
    /** Break active */
    unsigned int TxBREAK : 1;
    /**  Mark after Break Active */
    unsigned int TxMAB : 1;
    /** Indicates a transmission is complete (Last Byte loaded into buffer) */
//    unsigned int TxDone : 1;
    /** DMX_FLAGS Used to pass information from the ISR */
    unsigned int RDMNew : 1;
    
    unsigned int RDMcheck : 2;
    
    unsigned int RDMmute : 1;
    
} DMX_FLAGS;

/** DMX_Flags Set in the ISR to indicate to helper functions etc */
volatile DMX_FLAGS DMX_Flags;
//DMX set end


