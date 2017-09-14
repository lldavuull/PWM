
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
extern void timer1_init(void);
extern void timer1_switch(void);

enum {
    TIMER_500US,  //(500us)
    TIMER_BREAK,  //(90us)
    TX_TIMER_MAB,  //(20us)
//    TIMER_WAIT_TO_BREAK, //(180us)
//    TIMER_FILL  //(800us)
    TIMER_StartUpDelay, // (65.535*0x10=1048.56ms)
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

// 10us = 0xFFFF - 10 = 0xFFF5
#define TMR_LOAD_MAB     0xFFF5  // Load value for MAB      ( 10us)

// 180us = 0xFFFF - 180 = 0xFF4B
//#define TMR_LOAD_WAIT_TO_BREAK   0xFF4B  // Load Value for BREAK    (180us)

// 100us = 0xFFFF - 90 = 0xFFA5
#define TMR_LOAD_BREAK   0xFFA5  // Load Value for BREAK    (90us)

// 800us = 0xFFFF - 800 = 0xFCDF  - Adjust to fine tune the 1ms total
#define TMR_LOAD_FILL   0xFCDF   // Load value to total 1ms (800us)

//timer1 set end