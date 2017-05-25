/*
 * File:   DMX.c
 * Author: METEOR
 *
 * Created on May 23, 2017, 11:30 AM
 */


#include <xc.h>
#include "DMX.h"
#include "PWM.h"
#include <stdint.h>


/** RxData Receive Data Buffer  */
volatile char RxData[RX_BUFFER_SIZE];
/** RxChannel Base Address / Channel to start reading from */
int DMX_Address = 1;
/** RxStart The Start Code for a valid data stream */
char DMX_StartCode = 0;
/** AddrCount Address counter used in the interrupt routine */
volatile int RxAddrCount = 0;
/** *RxDataPtr Pointer to the receive data buffer. Used in interrupt */
volatile char *RxDataPtr;
/** RxState Current state in the receive state machine */
volatile char RxState = 0;
/** RxTimer Counts ms since last overflow - used for 1 second timeout */
volatile int RxTimer = 0;

enum {
    RX_WAIT_FOR_BREAK,
    WAIT_FOR_START,
    RX_DMX_READ_DATA,
    RX_RDM_READ_DATA,
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
    unsigned int TxDone : 1;

    /** DMX_FLAGS Used to pass information from the ISR */
} DMX_FLAGS;

/** DMX_Flags Set in the ISR to indicate to helper functions etc */
volatile DMX_FLAGS DMX_Flags;
//DMX set end

//RDM set start
/** RxStart The Start Code for a valid data stream */
char RDM_StartCode = 0xCC;

enum{
    SubStartCode,
    
};
//typedef struct{
//    struct{
//        uint16_t MANUFACTURE:16;
//        uint32_t ID:32;
//    }UID;
////    unsigned long int UID:48;       // this.device UID
////    unsigned long int CUID:48;   //Controller UID
//    unsigned TN:8;      //Transaction Number
//    unsigned PORT:8;     //Port ID / Response Type
//    unsigned message:8;     //message count
//    uint16_t subDevice:16;  //sub device
//    unsigned CC:8;          //Command Class
//    uint16_t PID:16;        //Parameter ID
//    unsigned PDL:8;         //Parameter Data Length
//    unsigned CSH:8;         //ChechSum High
//    unsigned CSL:8;         //ChechSum High
//}RDM_Data;

//volatile RDM_Data RX_RDM_Data;
//volatile RDM_Data TX_RDM_Data;


//RDM end start


//Timer1 set start
extern void timer1_interrupt(void);
extern void timer1_init(void);
enum {
    TIMER_1MS,
    TIMER_BREAK,
    TIMER_MAB,
    TIMER_FILL
};

volatile char TimerState = 0;

typedef struct {
    union {
        struct {
            unsigned int MS : 1;
            unsigned int SEC : 1;
            unsigned int MIN : 1;
            unsigned int HR : 1;
            unsigned int BREAK : 1; // Start BREAK Timer
            unsigned int MAB : 1; // Start MAB Timer
            unsigned int spare : 2;
        };
        char flags;
    };

    int MS_Count;
    char SEC_Count;
    char MIN_Count;
    char HR_Count;


} TIMER_DATA;

volatile TIMER_DATA Timer;

// 1ms = 0xFFFF - 1000 = 0xFc17
#define TMR1_LOAD_1MS     0xFc17  // Load value for 1ms
//// 20us = 0xFFFF - 20 = 0xFFEB
//#define TMR_LOAD_MAB     0xFFEB  // Load value for MAB      ( 20us)
//
//// 180us = 0xFFFF - 180 = 0xFF4B
//#define TMR_LOAD_BREAK   0xFF4B  // Load Value for BREAK    (180us)
//
//// 800us = 0xFFFF - 800 = 0xFCDF  - Adjust to fine tune the 1ms total
//#define TMR_LOAD_FILL   0xFCDF   // Load value to total 1ms (800us)

//timer1 set end


void DMX_init(void) {
        // DMX UART START
    RXPPS = 0b10101; //RX=RC5
    TRISC5 = 1; //set RC5 as input

    RCSTA = 0b10010000; //enable RX  ; 8bit
    SYNC = 0; // UART Enable
    BAUDCON = 0b00000000; //BRG16 =0
    BRGH = 1; //High Buad Speed
    SPBRGH = 0x00;
    SPBRGL = 0x3; //  16M/(16*(SPBRG+1)) = 250k
    RCIE = 1; //Enable RC interrupt
//    RxArPtr = &RxAr[0];
    // DMX UART END

    //    RC4PPS=0b1001; //RC4=TX
    //    TXSTA=0b01000101;//disable TX,
    timer1_init();
}


void DMX_loop(void) {
        if (DMX_Flags.RxNew == 1) {
            DMX_Flags.RxNew = 0;
            PWM1DCH = PWM.PWM[RxData[0]].DCH;
            PWM1DCL = PWM.PWM[RxData[0]].DCL;
            PWM2DCH = PWM.PWM[RxData[1]].DCH;
            PWM2DCL = PWM.PWM[RxData[1]].DCL;
            PWM3DCH = PWM.PWM[RxData[2]].DCH;
            PWM3DCL = PWM.PWM[RxData[2]].DCL;
            PWM4DCH = PWM.PWM[RxData[3]].DCH;
            PWM4DCL = PWM.PWM[RxData[3]].DCL;
            PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
        }

        if (Timer.MS) {
            Timer.MS = 0;
            // If no data received for 1200ms turn the lights off
            if (DMX_Flags.RxTimeout == 1) {
                PWM1DC =PWM2DC =PWM3DC =PWM4DC = 0;
                PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
            }
        }
}



void DMX_interrput(void) {
    timer1_interrupt();
    if (RCIE & RCIF) {
        RC3=~RC3;
        volatile char RxDat;
        if (FERR) // if get error bit, clear the bit ;  occur at space for "break"
        {
            RxDat = RCREG; // Clear the Framing Error - do not read before the bit test
            DMX_Flags.RxBreak = 1; // Indicate a break
            RxState = WAIT_FOR_START;
            RxTimer = 0;
        }

        switch (RxState) {
            case RX_WAIT_FOR_BREAK:
                RxDat = RCREG; // Just keep clearing the buffer until overflow.
                break;
            case WAIT_FOR_START:
                if (RCIF) // make sure there is data avaliable (ie not a break)
                {
                    RxDat = RCREG;
                    if (RxDat == DMX_StartCode) {   // DMX_StartCode == 00;
                        // Valid Start Received
                        RxState = RX_DMX_READ_DATA;
                        RxDataPtr = &RxData[0]; // Point to Buffer
                        RxAddrCount = 1; // Reset current addr - Start at address 1! (zero is OFF)
                        DMX_Flags.RxStart = 1; // Indicate a Start
                    } else if(RxDat == RDM_StartCode) { // RDM_StartCode == 0xCC;
                        // Valid Start Received
                        RxState = RX_RDM_READ_DATA;
                        RxDataPtr = &RxData[0]; // Point to Buffer
                        DMX_Flags.RxStart = 1; // Indicate a Start
                    }else{
                        RxState = RX_WAIT_FOR_BREAK;
                    }
                }
                break;
            case RX_DMX_READ_DATA:
                RxDat = RCREG;
                if (RxAddrCount >= DMX_Address && (DMX_Address != 0)) // A selection of channel zero is "OFF"
                {
                    *RxDataPtr = RxDat;
                    RxDataPtr++;
                }
                RxAddrCount++;
                // Check for end of data packet we can read
                if (RxAddrCount >= (DMX_Address + RX_BUFFER_SIZE) && DMX_Address != 0) {
                    DMX_Flags.RxNew = 1;
                    RxState = RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    DMX_Flags.RxTimeout = 0;
                }
                break;
            case RX_RDM_READ_DATA:
                
                break;
        }
        if (RxTimer > DMX_RX_TIMEOUT_MS) {
            DMX_Flags.RxTimeout = 1;
            RxTimer = 0;
        }
    }
}


void timer1_init(void){
    TMR1IE=1;
    T1CON = 0b00100001;          // Fosc/4, 1:4 pre  // 16MHz / 4 / 4 =  1us
}


void timer1_interrupt(void) {
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        switch (TimerState) {
            default:
                TimerState = TIMER_1MS;
            case TIMER_1MS:
                TMR1 = TMR1_LOAD_1MS;  // 0xFc17 Load value for 1ms,  ( 0xFFFF-0xFc17 = 0x02E8 = 1000us )

                RxTimer++; // Inc timeout counter for receiver
                if (RxTimer == DMX_RX_TIMEOUT_MS) {
                    RxTimer = DMX_RX_TIMEOUT_MS + 1;
                    DMX_Flags.RxTimeout = 1;
                }


                Timer.MS_Count++; // Inc the MS Counter
                Timer.MS = 1; // Set the ms flag
                if (Timer.MS_Count == 1000) // Check for 1 second
                {
                    Timer.MS_Count = 0;
                    Timer.SEC_Count++;
                    Timer.SEC = 1;
                    if (Timer.SEC_Count == 60) // Check for Minute
                    {
                        Timer.SEC_Count = 0;
                        Timer.MIN_Count++;
                        Timer.MIN = 1;

                        if (Timer.MIN_Count == 60) // Check for Hour
                        {
                            Timer.MIN_Count = 0;
                            Timer.HR_Count++;
                            Timer.HR = 1;
                        }
                    }
                }
                break;
        }
    }
}