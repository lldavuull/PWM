/*
 * File:   main.c
 * Author: METEOR
 *
 * Created on 2017?5?5?, ?? 10:37
 */


#include <xc.h>
#include "DMX.h"
#include "PWM.h"
#include "PFM.h"
#include "RDM.h"
#include "Timer.h"
__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & BOREN_OFF & CLKOUTEN_OFF);
__CONFIG(WRT_HALF & PPS1WAY_OFF & PLLEN_ON & STVREN_ON & LPBOREN_OFF & LVP_OFF);

#define PWMxCON_SET  0b10000000        // Enhanced features off
#define PRx_SET      0xFF              // Max resolution

void timer_interrupt(void);

void main(void) {
    PWM1DCH = 0x00;
    PWM2DCH = 0x00;
    PWM3DCH = 0x00;
    PWM4DCH = 0x00;

    PWM1DCL = 0x00;
    PWM2DCL = 0x00;
    PWM3DCL = 0x00;
    PWM4DCL = 0x00;
    PWM1LDCON=PWM2LDCON=PWM3LDCON=PWM4LDCON=0b10000000;
    
    RA2PPS = 0b0011; //PWM1_out
    RC0PPS = 0b0100; //PWM2_out
    RC1PPS = 0b0101; //PWM3_out
    RC2PPS = 0b0110; //PWM4_out
    
    PR2 = PRx_SET;

    OSCCON = 0b11110000; // 4xPLL,32MHz, Config bits determine source  //   PLL=Phase-locked
    OSCTUNE = 0b000000;
    
    TRISA2 = TRISC0 = TRISC1 = TRISC2 = 0; //RA2, RC0, RC1, RC2 set to output
    ANSA2 = ANSC0 = ANSC1 = ANSC2 = 0; //RA2, RC0, RC1, RC2 set to I/O
    
//    RA2=RC0=RC1=RC2=0;
    PWM1PHH = PWM2PHH = PWM3PHH = PWM4PHH = 0x00;
    PWM1PHL = PWM2PHL = PWM3PHL = PWM4PHL = 0x00;
    PWM1PRH = PWM2PRH = PWM3PRH = PWM4PRH = 0xFF;
    PWM1PRL = PWM2PRL = PWM3PRL = PWM4PRL = 0xFF;
    PWM1CLKCON = PWM2CLKCON = PWM3CLKCON = PWM4CLKCON = 0b00000000; //1:1
    
    
    TRISC3 = 0;
    ANSC3 = 0; //RC3  is for interrupt test
    INTCON = 0b11000000; //GIE=1; TMR0 interrupt=0; PEIE interrupt=1;
    
//    DMX_Address = 1;
    timer_init();
//    ADC_init();
    DMX_init();
    RDM_init();
   
    PWM1CON = PWM2CON = PWM3CON = PWM4CON = PWMxCON_SET;
    
    
    
    
//    TX_PD_Flag=0;
    
    while (1) {
        DMX_loop();
//        ADC_loop();
//        timer1_switch();
        RDM_rx_loop();
        timer2_loop();
    }
}

void interrupt isr(void) {
//    ADC_interrupt();
    DMX_interrput();
    timer1_interrupt();
    RDM_tx_interrupt();
    timer2_interrupt();
//    PWM_Level_interrupt();
}
                
//                *RxArPtr=RxAddrCount;
//                RxArPtr++;
//                *RxArPtr=DMX_Address;
//                RxArPtr++;
//                *RxArPtr=DMX_Address + RX_BUFFER_SIZE;
//                RxArPtr++;
//                i++;




