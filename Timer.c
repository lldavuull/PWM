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
    T1CON    = 0b00110001; // Fosc/4, 1:8 pre  // 32MHz / 4 / 8 =  1us
//    TimerState = TIMER_StartUpDelay;
//    TMR1=0;
//    Timer_DelayCount=0;
}
#define PWMxCON_SET  0b10000000        // Enhanced features off

void timer1_interrupt(void) {
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        switch (TimerState) {
            default:
                TimerState = TIMER_500US;
            case TIMER_StartUpDelay:
                TMR1=0;
                Timer_DelayCount++;
                if(Timer_DelayCount>=0x10){
//                    TimerState = TIMER_500US;
//                    ADC_init();
//                    DMX_init();
//                    RDM_init();
//                    PWM1CON = PWM2CON = PWM3CON = PWM4CON = PWMxCON_SET;
                }
                break;
            case TIMER_500US:
//                if (Timer.BREAK == 1 && DMX_Flags.TxRunning == 1) // Check if sending a new BREAK
//                {
//                    TMR1 = TMR_LOAD_BREAK; //=0xFF4B  Load Value for BREAK    (180us)
//                    TX_PIN = 0;
//                    TimerState = TIMER_BREAK;
//                    Timer.BREAK = 0;
//                } 
//                else {
                    if(ADIE==RA5){ // RA5==0 or RA5==1; => switch to ADIE=~RA5, RCIE=RA5;
                        Timer.Switch=1;
                    }
                    TMR1 = TMR1_LOAD_500US;
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
//                }
                break;
//            case TIMER_WAIT_TO_BREAK:
//                TX_PIN = 0; //set low ?SPACE? for BREAK
//                RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//                TimerState = TX_TIMER_MAB; // Next state is MAB end
//                break;
                
            case TIMER_BREAK:
                TX_PIN = 1; // Set pin high for MAB
                TMR1 = TMR_LOAD_MAB; // Load the MAB time = 0xFFF5  // Load value for MAB      ( 10us)
                TimerState = TX_TIMER_MAB; // Next state is MAB end
                break;

            case TX_TIMER_MAB:
                TXEN = 1; // Re-enable EUSART control of pin    
                TXIE = 1; // Re-Enable EUSART Interrupt     , it will send RDM_StartCode (0xCC)
                TMR1 = TMR_LOAD_FILL; // Load the Filler time = 0xFCDF   // Load value to total 1ms (800us)
                TimerState = TIMER_500US; // Next int is the 0.5ms
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