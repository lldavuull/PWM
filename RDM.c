/*
 * File:   RDM.c
 * Author: METEOR
 *
 * Created on 2017?5?26?, ?? 4:29
 */


#include <xc.h>
#include "RDM.h"
#include "DMX.h"
#include "Timer.h"
void RDM_rx_loop(void){
    if(DMX_Flags.RDMNew==1){
        DMX_Flags.RDMNew=0;
        //1. checksum
        checkSum=0x00CD;   // 0xCC+ 0x01
        PackCount=0;
        while(PackCount<22){
            checkSum+=RX_RDM_Data.value[PackCount];
            PackCount++;
        }
        if(checkSum==RX_RDM_Data.CS){
            RC3=~RC3;
        }else{
        }
        //2. 
    }
}

void RDM_init(void) {
    TimerState = 0;

    Timer.HR_Count = 0;
    Timer.MIN_Count = 0;
    Timer.SEC_Count = 0;
    Timer.flags = 0;

    // Set up the Timer for 1ms Interrupts
    TMR1 = TMR1_LOAD_1MS;
    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1; // Turn the peripheral interrupt ON

    // Set up EUSART options for TX
    TX_PIN = 1; // Set pin (RC4) used high
    TX_TRIS = 0; // Set TX pin (RC4) as output
//    TXIE = 1; // Only Transmit when ready
}

void RDM_tx_interrupt(void) {
    if (TXIE && TXIF) {
        switch (TxState) {
            case TX_BREAK:
                TX_PIN = 1; // Ensure PIN will be high
                TXEN = 0; // Disable the EUSART's control of the TX pin
                Timer.BREAK = 1; // Indticate a BREAK to the timer interrupt
                TXIE = 0; // Disable the EUSART Interrupt 
                TxState = TX_START; // Move to Start code sequence
                break;

            case TX_MAB:
                break;

            case TX_START: // Send start and set up for data stream
                TXREG = TxStart;
                //                TxByte = &TxData[0]; // Point to First byte
                TxCount = 0; // Clear the address counter
                TxState = TX_DATA;
                break;

            case TX_DATA: // Send the data
                if (TxCount < TX_BUFFER_SIZE) {
                    TXREG = *TxByte;
                    TxByte++; // Point to next byte
                    TxCount++; // Count the addresses
                } else {
                    TXREG = 0; // Put one more byte in the buffer to force next interrupt
                    TxState = TX_BREAK; // Next interrupt go to BREAK
                    DMX_Flags.TxDone = 1;
                }
                break;
        }
    }
}