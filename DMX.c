/*
 * File:   DMX.c
 * Author: METEOR
 *
 * Created on May 23, 2017, 11:30 AM
 */


#include <xc.h>
#include "DMX.h"
#include "PWM.h"
#include "RDM.h"
#include "Timer.h"
#include <stdint.h>

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
    RDM_init();
    DMX_Address=1;
}

void DMX_loop(void) {
    
    //DMX
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
    RDM_rx_loop();
    if (Timer.MS) {
        Timer.MS = 0;
        // If no data received for 1200ms turn the lights off
        if (DMX_Flags.RxTimeout == 1) {
            PWM1DC = PWM2DC = PWM3DC = PWM4DC = 0;
            PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
        }
    }
}


void DMX_interrput(void) {
//    RDM_tx_interrupt();
    if (RCIE & RCIF ) {
        
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
                    if (RxDat == DMX_StartCode) { // DMX_StartCode == 00;
                        // Valid Start Received
                        RxState = RX_DMX_READ_DATA;
                        RxDataPtr = &RxData[0]; // Point to Buffer
                        RxAddrCount = 1; // Reset current addr - Start at address 1! (zero is OFF)
                        DMX_Flags.RxStart = 1; // Indicate a Start
                    } else if (RxDat == RDM_StartCode) { // RDM_StartCode == 0xCC;
                        // Valid Start Received
                        RxState = RX_RDM_READ_SubStartCode;
                        DMX_Flags.RxStart = 1; // Indicate a Start
                    } else {
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
            case RX_RDM_READ_SubStartCode:
                RxDat = RCREG;
                if (RxDat == RDM_SubStartCode) // RDM_SubStartCode == 0x01;
                {
                    RxState = RX_RDM_READ_DATA;
                    PackCount=0;
                    PD_Flag=0;
                }
                break;

            case RX_RDM_READ_DATA:
                RxDat = RCREG;
                RX_RDM_Data.value[PackCount]=RxDat;
                PackCount++;

                if (PackCount==23 && RX_RDM_Data.PDL>0 && PD_Flag==0) {
                    RxState=RX_RDM_PD;
                    PDCount=0;
                    break;
                }

                if(PackCount==24){
                    DMX_Flags.RDMNew = 1;
                    RxState = RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    DMX_Flags.RxTimeout = 0;
                }
                break;
            case RX_RDM_PD:
                RxDat = RCREG;
                PD[PDCount]=RxDat;
                PDCount++;
                if(RX_RDM_Data.PDL==PDCount){
                    RxState=RX_RDM_READ_DATA;
                    PackCount++;
                    PD_Flag=1;
                }
                break;
        }
        if (RxTimer > DMX_RX_TIMEOUT_MS) {
            DMX_Flags.RxTimeout = 1;
            RxTimer = 0;
        }
    }
}


void dmx_write(uint16_t addr, uint8_t *data, uint8_t num) {
    if (addr <= TX_BUFFER_SIZE && addr != 0) {
        addr--; // Adjust offset to match 0-511 range for buffer
        GIE = 0; // Go Atomic ##### SHOULD JUST DISABLE TXIE
        for (; num--; num > 0) {
            //           TxData[addr]=*data;
            addr++;
            data++;
            if (addr >= TX_BUFFER_SIZE)num = 0;
        }
        GIE = 1; // End Atomic
    }
}