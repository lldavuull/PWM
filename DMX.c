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

    DMX_Address = 1;

    PWMDCLptr[0] = &PWM1DCL;
    PWMDCLptr[1] = &PWM2DCL;
    PWMDCLptr[2] = &PWM3DCL;
    PWMDCLptr[3] = &PWM4DCL;
    PWMDCHptr[0] = &PWM1DCH;
    PWMDCHptr[1] = &PWM2DCH;
    PWMDCHptr[2] = &PWM3DCH;
    PWMDCHptr[3] = &PWM4DCH;
    PWMLDCONptr[0] = &PWM1LDCON;
    PWMLDCONptr[1] = &PWM2LDCON;
    PWMLDCONptr[2] = &PWM3LDCON;
    PWMLDCONptr[3] = &PWM4LDCON;
}

void DMX_loop(void) {
    char Addr = 0;

    //DMX Renew
    if (DMX_Flags.RxNew == 1) {
        DMX_Flags.RxNew = 0;

        //Smooth PWM
        while (Addr < 4) {
            if (RGBW[Addr] > RxData[Addr]) {
                DMXPeriodSign[Addr].X = 0b10;//decrease
                if ((RGBW[Addr] - RxData[Addr]) < DMXPeriod) {
                    DMXPeriodStep[Addr] = DMXPeriod / (RGBW[Addr] - RxData[Addr]);
                } else {
                    DMXPeriodStep[Addr] = 1;
                }
                DMXPeriodStepConst[Addr]=DMXPeriodStep[Addr];
            } else if (RGBW[Addr] < RxData[Addr]) {
                DMXPeriodSign[Addr].X = 0b01;//increase
                if ((RxData[Addr] - RGBW[Addr]) < DMXPeriod) {
                    DMXPeriodStep[Addr] = DMXPeriod / (RxData[Addr] - RGBW[Addr]);
                } else {
                    DMXPeriodStep[Addr] = 1;
                }
                DMXPeriodStepConst[Addr]=DMXPeriodStep[Addr];
            } else {    //unchage
                DMXPeriodSign[Addr].X = 0b00;
                DMXPeriodStep[Addr] = 255;
            }
            Addr++;
        }
    }

    //DMX didn't get Signal
    if (Timer.MS) {
        DMXPeriodConst++;
        Timer.MS = 0;
        

        Addr = 0;
        while (Addr < 4) {
            switch (DMXPeriodSign[Addr].X) {
                case 0b01:
                    if(DMXPeriodStepConst[Addr]==0){
                        DMXPeriodStepConst[Addr]=DMXPeriodStep[Addr];
                        RGBW[Addr]++;
                    }
                    DMXPeriodStepConst[Addr]--;
                    break;
                case 0b10:
                    if(DMXPeriodStepConst[Addr]==0){
                        DMXPeriodStepConst[Addr]=DMXPeriodStep[Addr];
                        RGBW[Addr]--;
                    }
                    DMXPeriodStepConst[Addr]--;
                    break;
                case 0b00:
                    Addr++;
                    continue;
                    break;
            }
            *PWMDCHptr[Addr] = PWM.PWM[RGBW[Addr]].DCH;
            *PWMDCLptr[Addr] = PWM.PWM[RGBW[Addr]].DCL;
            *PWMLDCONptr[Addr] = 0b10000000;
            if (RGBW[Addr] == RxData[Addr]) {
                DMXPeriodSign[Addr].X = 0b00;
            }
            Addr++;
        }

//        TXREG = RGBW[0];
//        TXIE = 1;
        
        // If no data received for 1200ms turn the lights off
        if (DMX_Flags.RxTimeout == 1) {
            PWM1DC = PWM2DC = PWM3DC = PWM4DC = 0;
            PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
            Addr=0;
            while (Addr < 4) {
                DMXPeriodSign[Addr].X=0b00;
                Addr++;
            }
        }
    }
}

void DMX_interrput(void) {

    if (RCIE & RCIF) {

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
                        DMXPeriod = DMXPeriodConst; // DMX period Record;
                        DMXPeriodConst = 0; // DMX period reset;
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
                    PackCount = 23;
                    PD_Flag = 0;
                }
                break;

            case RX_RDM_READ_DATA:
                RxDat = RCREG;
                RX_RDM_Data.value[PackCount] = RxDat;

                if (PackCount == 2 && RX_RDM_Data.PDL > 0 && PD_Flag == 0) {
                    RxState = RX_RDM_PD;
                    PDCount = 230;
                    break;
                }

                if (PackCount == 0) {
                    DMX_Flags.RDMNew = 1;
                    RxState = RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    DMX_Flags.RxTimeout = 0;
                }

                PackCount--;
                break;
            case RX_RDM_PD:
                RxDat = RCREG;
                PD[PDCount] = RxDat;
                PDCount--;
                if ((PD_LEN - RX_RDM_Data.PDL) == PDCount) {
                    RxState = RX_RDM_READ_DATA;
                    PD_Flag = 1;
                }
                break;
        }
        if (RxTimer > DMX_RX_TIMEOUT_MS) {
            DMX_Flags.RxTimeout = 1;
            RxTimer = 0;
        }
    }
}


//void dmx_write(uint16_t addr, uint8_t *data, uint8_t num) {
//    if (addr <= TX_SIZE && addr != 0) {
//        addr--; // Adjust offset to match 0-511 range for buffer
//        GIE = 0; // Go Atomic ##### SHOULD JUST DISABLE TXIE
//        for (; num--; num > 0) {
//            //           TxData[addr]=*data;
//            addr++;
//            data++;
//            if (addr >= TX_SIZE)num = 0;
//        }
//        GIE = 1; // End Atomic
//    }
//}