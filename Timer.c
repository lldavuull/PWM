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

void timer_init(void) {
    
    TMR1IE = 1;
    T1CON    = 0b00110001; // Fosc/4, 1:8 pre  // 32MHz / 4 / 8 =  1us
    
    TimerState = TIMER_500US;

    Timer.HR_Count = 0;
    Timer.MIN_Count = 0;
    Timer.SEC_Count = 0;
    Timer.flags = 0;

    // Set up the Timer for 1ms Interrupts
    TMR1 = TMR1_LOAD_500US;
    TMR1IF = 0;
    TMR1IE = 1;
    
    
    //Timer2 use for RDM_identify
//    TMR2IE=1;  //turn on while RDM_identify;
    T2CON=0b01001111;// Fosc/4, 1:64 pre, 1:10post  // 32MHz / 4 / 64 /10 =  80us
    PR2=0xf9; //0xf9=249, 80us*250 = 20ms
}


void timer1_interrupt(void) {
    if (TMR1IE && TMR1IF) {
        TMR1IF = 0;
        switch (TimerState) {
            default:
                TimerState = TIMER_500US;
//            case TIMER_StartUpDelay:
//                TMR1=0;
//                Timer_DelayCount++;
//                if(Timer_DelayCount>=0x10){
////                    TimerState = TIMER_500US;
////                    ADC_init();
////                    DMX_init();
////                    RDM_init();
////                    PWM1CON = PWM2CON = PWM3CON = PWM4CON = PWMxCON_SET;
//                }
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
//                    if(ADIE==RA5){ // RA5==0 or RA5==1; => switch to ADIE=~RA5, RCIE=RA5;
//                        Timer.Switch=1;
//                    }
                    TMR1 = TMR1_LOAD_500US;
                    RxTimer++; // Inc timeout counter for receiver
                    if (RxTimer == DMX_RX_TIMEOUT_MS) {
                        RxTimer = DMX_RX_TIMEOUT_MS + 1;
                        DMX_Flags.RxTimeout = 1;
                    }

                    Timer.MS_Count++; // Inc the MS Counter
                    Timer.MS = 1; // Set the ms flag
                    if (Timer.MS_Count == 1000) // Check for 0.5 second
                    {
//                        if(DMX_Flags.RDM_Identify_Device){
//                            DMX_Flags.RDM_Identify_Device_Switch=~DMX_Flags.RDM_Identify_Device_Switch;//1= On, 0=Off; Period = 1s
//                            if(DMX_Flags.RDM_Identify_Device_Switch){
//                                CurrentPWM.DC=0xffff;
//                            }else{
//                                CurrentPWM.DC=0;
//                            }
//                            while (Addr < 4) {
//                                *PWMDCptr[Addr] = CurrentPWM.DC;
//                                *PWMLDCONptr[Addr] = 0b10000000; 
//                                Addr++;
//                            }
//                        }

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
            case TIMER_RDM_MBB:
                TXEN = 0;
                TMR1 = TMR_LOAD_RDM_BREAK; // Load Value for BREAK    (180us)
                TimerState = TIMER_RDM_BREAK; // Next state is MAB end
                break;
            case TIMER_RDM_BREAK:
                TXEN = 1;
                TMR1 = TMR_LOAD_RDM_MAB; // Load the MAB time = 0xFFF5  // Load value for MAB      ( 10us)
                TimerState = TIMER_RDM_MAB; // Next state is MAB end
                break;

            case TIMER_RDM_MAB:
                TMR1 = TMR_LOAD_FILL; // Load the Filler time = 0xFCDF   // Load value to total 1ms (800us)
                TimerState = TIMER_500US; // Next int is the 0.5ms
//                RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//                TXEN = 1; // Re-enable EUSART control of pin    
                TXIE = 1; // Re-Enable EUSART Interrupt     , it will send RDM_StartCode (0xCC)
                break;
            case TIMER_DISC_MAB:
//                TXEN = 1; // Re-enable EUSART control of pin    
                TXIE = 1; // Re-Enable EUSART Interrupt     , it will send RDM_StartCode (0xCC)
                TMR1 = TMR_LOAD_FILL; // Load the Filler time = 0xFCDF   // Load value to total 1ms (800us)
                TimerState = TIMER_500US; // Next int is the 0.5ms
//                RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
                break;
                
                

//            case TX_TIMER_MAB_DISCOVERY:
//                TX_PIN = 1; // Set pin high for MAB
////                RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//                TMR1 = TMR_LOAD_MAB; // Load value for MAB 
//                TimerState = TX_TIMER_MAB; // Next int is the 0.5ms
//                break;
                
            case TIMER_MAS:
                TMR1 = TIMER_500US; // Load the Filler time = 0xFCDF   // Load value to total 1ms (800us)
                TimerState = TIMER_500US; // Next int is the 0.5ms
//                TX_PIN = 1;
//                TXEN = 0; // Disable the EUSART's control of the TX pin
                RXTX_SWITCH_PIN=0;
                break;
                
//            case TX_TEST:
//                LATC3 = 0; //Set switch pin to TX mode
//                TimerState = TIMER_500US; // Next int is the 0.5ms
//                break;
        }
    }
}

void timer2_interrupt(void) {
    if (TMR2IE && TMR2IF) { //Preriod 20ms
        TMR2IF = 0;
        Timer2_Count++;
        if(Timer2_Count==50){ //20ms * 50 = 1s
            Timer2_Count=0;
            DMX_Flags.RDM_Identify_Device_Timer2New = 1;
        }
    }
}
void timer2_loop(void){
    if(DMX_Flags.RDM_Identify_Device_Timer2New){
        DMX_Flags.RDM_Identify_Device_Timer2New=0;
        DMX_Flags.RDM_Identify_Device_Flash=~DMX_Flags.RDM_Identify_Device_Flash;
        if(DMX_Flags.RDM_Identify_Device_Flash){
            PWM_TurnOn();
        }else{
            PWM_TurnOff();
        }
    }
}

void RDM_Identify_Switch(void){
    DMX_Flags.RDM_Identify_Device=~DMX_Flags.RDM_Identify_Device;
    TMR2IE=DMX_Flags.RDM_Identify_Device;
    if(!DMX_Flags.RDM_Identify_Device){
        PWM_TurnOff();
    }
}

    
//void timer1_switch(void){
//    if(Timer.Switch==1){
//        SPEN=RA5;
//        RC3=RA5;
//        RCIE=RA5;
//        volatile char RxDat2;
//        Timer.Switch=0;
//        ADIE=~RA5;
//        ADON=~RA5;
//        GO_nDONE = ~RA5;
//        if (FERR){}
//        RxDat2 = RCREG;
//        ADIF = 0;
//        DMX_Flags.RxTimeout = 0;
//        RxState = WAIT_FOR_START;
//    }
//}