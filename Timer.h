
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h>
#include <stdint.h>


//Timer1 set start
extern void timer1_interrupt(void);
extern void timer_init(void);
extern void timer1_switch(void);
extern void RDM_Identify_Switch(void);
extern void timer2_loop(void);

enum {
    TIMER_500US,  //(500us)
    TIMER_RDM_BREAK,  //(180us)
    TIMER_RDM_MAB,  //(10us)
    TIMER_RDM_MBB,
//    TIMER_DISC_MARK,  //(10us)
//    TIMER_WAIT_TO_BREAK, //(180us)
//    TIMER_FILL  //(800us)
//    TIMER_StartUpDelay, // (65.535*0x10=1048.56ms)
    TIMER_MAS, //(4us)
    TIMER_DISC_MAB, //(4us)
    
//    TX_TEST
};
volatile char TimerState = 0;

volatile char Timer_DelayCount = 0; // 0x0~ 0x10 

typedef struct {
    union {
        struct {
            unsigned int MS : 1;
            unsigned int SEC : 1;
            unsigned int MIN : 1;
            unsigned int HR : 1;
//            unsigned int BREAK : 1; // Start BREAK Timer
            unsigned int MAB : 1; // Start MAB Timer
            unsigned int spare : 2;
        };
        char flags;
    };
    int MS_Count;
    char SEC_Count;
    char MIN_Count;
    char HR_Count;
    unsigned int Switch : 1;
} TIMER_DATA;

volatile TIMER_DATA Timer;

// 1ms = 0xFFFF - 1000 = 0xFC17
// 500us = 0xFFFF - 500 = 0xFE0B
#define TMR1_LOAD_500US     0xFE0B  // Load value for 0.5ms   

//1MS     0xFc17

//0xFFFF - 40 = 0xffd7

#define TMR_LOAD_RDM_MBB     0xFFFF  // Load value for MBB      (10us)

// 140us = 0xFFFF - 176 = 0xFF4f
#define TMR_LOAD_RDM_BREAK   0xFF4f  // Load Value for BREAK    (176us)
//#define TMR_LOAD_RDM_BREAK   0xFFA5  // Load Value for BREAK    (90us)

// 10us = 0xFFFF - 10 = 0xFFF5
#define TMR_LOAD_RDM_MAB     0xFFF5  // Load value for MAB      (10us)

// 800us = 0xFFFF - 800 = 0xFCDF  - Adjust to fine tune the 1ms total
#define TMR_LOAD_FILL   0xFCDF   // Load value to total 1ms (800us)

// 4us = 0xFFFF - 50 = 0xFFCD  
#define TMR_LOAD_MAS   0xFFDD   // Load value for MAB      ( 50us)

// 8us = 0xFFFF - 8 = 0xFFF7
#define TMR_LOAD_DISC_MAB   0xFFF5   // Load value for MAB Discovery      ( 10us)
//timer1 set end

char Timer2_Count = 0; // 0x0~ 0x10 

// 4us = 0xFFFF - 4 = 0xFFFB  - Adjust to fine tune the 1ms total

// 3.2.5 Discovery Response MARK time
// Responders may wish to drive a MARK on the line for at least 4?s prior to the first start bit of the discovery response.
// This will ensure that all other devices on the line recognize the start bit.

//#define TMR_DISC_MARK   0xFFF5   // Load value to total 10us

