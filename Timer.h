
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
//    TIMER_BREAK,  //(180us)
    TIMER_MAB,  //(20us)
    TIMER_FILL  //(800us)
};

volatile char TimerState = 0;

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

// 1ms = 0xFFFF - 1000 = 0xFc17
#define TMR1_LOAD_500US     0xFE0B  // Load value for 1ms   

//1MS     0xFc17

// 20us = 0xFFFF - 20 = 0xFFEB
#define TMR_LOAD_MAB     0xFFEB  // Load value for MAB      ( 20us)

// 180us = 0xFFFF - 180 = 0xFF4B
#define TMR_LOAD_BREAK   0xFF4B  // Load Value for BREAK    (180us)

// 800us = 0xFFFF - 800 = 0xFCDF  - Adjust to fine tune the 1ms total
#define TMR_LOAD_FILL   0xFCDF   // Load value to total 1ms (800us)

//timer1 set end