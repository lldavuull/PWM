/*
 * File:   Timer.c
 * Author: METEOR
 *
 * Created on 2017?5?31?, ?? 10:25
 */


#include <xc.h>
#include "Timer.h"
#include "DMX.h"
#include "PWM.h"
#include "RDM.h"

void timer1_init(void) {
    TMR1IE = 1;
    T1CON = 0b00100001; // Fosc/4, 1:4 pre  // 16MHz / 4 / 4 =  1us
}


void timer1_interrupt(void) {
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        
        switch (TimerState) {
            default:
                TimerState = TIMER_1MS;
            case TIMER_1MS:
                
                if (Timer.BREAK == 1 && DMX_Flags.TxRunning == 1) // Check if sending a new BREAK
                {
                    TMR1 = TMR_LOAD_BREAK; //=0xFF4B  Load Value for BREAK    (180us)
                    TX_PIN = 0;
                    TimerState = TIMER_BREAK;
                    Timer.BREAK = 0;
                } else {
                    if(ADIE==RA5){ // RA5==0 or RA5==1; => switch to ADIE=~RA5, RCIE=RA5;
                        Timer.Switch=1;
                    }
                    TMR1 = TMR1_LOAD_1MS;
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
                }
                break;

            case TIMER_BREAK:
                TX_PIN = 1; // Set pin high for MAB
                TMR1 = TMR_LOAD_MAB; // Load the MAB time = 0xFFEB  // Load value for MAB      ( 20us)
                TimerState = TIMER_MAB; // Next state is MAB end
                break;

            case TIMER_MAB:
                TXEN = 1; // Re-enable EUSART control of pin
                TXIE = 1; // Re-Enable EUSART Interrupt
                TMR1 = TMR_LOAD_FILL; // Load the Filler time = 0xFCDF   // Load value to total 1ms (800us)
                TimerState = TIMER_1MS; // Next int is the 1ms
                break;
        }
    }
}

void timer1_switch(void){
    if(Timer.Switch==1){
        SPEN=RA5;
        RC3=RA5;
        RCIE=RA5;
        volatile char RxDat2;
        Timer.Switch=0;
        ADIE=~RA5;
        ADON=~RA5;
        GO_nDONE = ~RA5;
        if (FERR){}
        RxDat2 = RCREG;
        ADIF = 0;
        DMX_Flags.RxTimeout = 0;
        RxState = WAIT_FOR_START;
    }
}