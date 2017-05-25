/*
 * File:   main.c
 * Author: METEOR
 *
 * Created on 2017?5?5?, ?? 10:37
 */


#include <xc.h>
#include "DMX.h"
#include "PWM.h"




__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & BOREN_OFF & CLKOUTEN_OFF);
__CONFIG(WRT_OFF & PPS1WAY_OFF & PLLEN_ON & STVREN_ON & LPBOREN_OFF & LVP_OFF);

#define PWMxCON_SET  0b10000000        // Enhanced features off
#define PRx_SET      0xFF              // Max resolution
#define TxCON_SET    0b00000101        // Post=1:1, ON, PRE=1:4

void timer_interrupt(void);

void main(void) {
    PWM1CON = PWM2CON = PWM3CON = PWM4CON = PWMxCON_SET;
    PR2 = PRx_SET;
    T2CON = TxCON_SET;

    OSCCON = 0b11111000; // 4xPLL,16MHz, Config bits determine source  //   PLL=Phase-locked
    OSCTUNE = 0b000000;

    TRISA2 = TRISC0 = TRISC1 = TRISC2 = 0; //RA2, RC0, RC1, RC2 set to output
    ANSA2 = ANSC0 = ANSC1 = ANSC2 = 0; //RA2, RC0, RC1, RC2 set to I/O

    RA2PPS = 0b0011; //PWM1_out
    RC0PPS = 0b0100; //PWM2_out
    RC1PPS = 0b0101; //PWM3_out
    RC2PPS = 0b0110; //PWM4_out

    PWM1DCH = 0x0F;
    PWM2DCH = 0x0C;
    PWM3DCH = 0x09;
    PWM4DCH = 0x06;

    PWM1DCL = 0xFF;
    PWM2DCL = 0x7E;
    PWM3DCL = 0x0D;
    PWM4DCL = 0x9C;

    PWM1PHH = PWM2PHH = PWM3PHH = PWM4PHH = 0x00;
    PWM1PHL = PWM2PHL = PWM3PHL = PWM4PHL = 0x00;
    PWM1PRH = PWM2PRH = PWM3PRH = PWM4PRH = 0x1F;
    PWM1PRL = PWM2PRL = PWM3PRL = PWM4PRL = 0xFF;
    PWM1CLKCON = PWM2CLKCON = PWM3CLKCON = PWM4CLKCON = 0b00000000; //1:1

    
    TRISC3 = 0;
    ANSC3 = 0; //RC3  is for interrupt test
    INTCON = 0b11000000; //GIE=1; TMR0 interrupt=0; PEIE interrupt=1;
    
    
    
    
    DMX_init();
//    Sweep_PWM_init();
//    ADC_init();
    


    while (1) {
        DMX_loop();
//        ADC_loop();

    }
}

void interrupt isr(void) {
    
//    ADC_interrupt();
//        Sweep_PWM();
        DMX_interrput();
    //    timer_interrupt();
}






//                RC3=~RC3;
//                *RxArPtr=RxAddrCount;
//                RxArPtr++;
//                *RxArPtr=DMX_Address;
//                RxArPtr++;
//                *RxArPtr=DMX_Address + RX_BUFFER_SIZE;
//                RxArPtr++;
//                i++;




