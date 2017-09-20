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
#include "rdm_define.h"

void DMX_init(void) {
    // DMX UART START
    RXPPS = 0b10101; //RX=RC5
    TRISC5 = 1; //set RC5 as input

    RCSTA = 0b10010000; //enable RX  ; 8bit
    SYNC = 0; // UART Enable
    BAUDCON = 0b00000000; //BRG16 =0
    BRGH = 1; //High Buad Speed
    SPBRGH = 0x00;
    SPBRGL = 0x7; //  32M/(16*(SPBRG+1)) = 250k
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
    
    PWM_Pin[0]=RA2;
    PWM_Pin[1]=RC0;
    PWM_Pin[2]=RC1;
    PWM_Pin[3]=RC2;
    
    char Addr=0;
    while(Addr<4){
        DMX_sumRepeat[Addr]=2;
        DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
        DMXSign[Addr].InfiniteLoop=0;
        DMX_TargetBright[Addr]=DMX_CurrentBright[Addr]=0.0;
        Addr++;
    }
}
void DMX_loop(void) {
    char Addr = 0;
    //DMX Renew
    if (DMX_Flags.RxNew == 1) {
        DMX_Flags.RxNew = 0;
            //////////
        DMXPeriodDimming=0;
        DMXStepConst=DMXStep;
        while (Addr < 4) {
            rxdata=RxData[Addr];  //avoid violatile to acculate.
            
            if (DMX_TargetBright[Addr] < rxdata) {
                DMX_difference=rxdata-DMX_TargetBright[Addr];
                if(DMX_difference<0.05){
                    DMXSign[Addr].SIGN = 0b00;//increase
                    DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                }else{
                    DMXSign[Addr].SIGN = 0b01;//increase
                    DMX_CurrentBright[Addr]=DMX_TargetBright[Addr];
                    DMX_TargetBright[Addr]=DMX_TargetBright[Addr]+(DMX_difference*2/DMX_sumRepeat[Addr]);
                    DMX_SpaceBright[Addr]=(DMX_TargetBright[Addr]-DMX_CurrentBright[Addr])/(DMXPeriod>>DMX_Shift_bits);
                }
            } else if (DMX_TargetBright[Addr] > rxdata) {
                DMX_difference=DMX_TargetBright[Addr]-rxdata;
                if(DMX_difference<0.05){
                    DMXSign[Addr].SIGN = 0b00;//increase
                    DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                }
                else{
                    DMXSign[Addr].SIGN = 0b10;//decrease
                    DMX_CurrentBright[Addr]=DMX_TargetBright[Addr];
                    DMX_TargetBright[Addr]=DMX_TargetBright[Addr]-(DMX_difference*2/DMX_sumRepeat[Addr]);
                    DMX_SpaceBright[Addr]=(DMX_CurrentBright[Addr]-DMX_TargetBright[Addr])/(DMXPeriod>>DMX_Shift_bits);
                }
            } else {    //unchange
                DMX_CurrentBright[Addr]=DMX_TargetBright[Addr]=rxdata;
                DMXSign[Addr].SIGN = 0b00;
            }
            
            if(preRxData[Addr]==RxData[Addr]){
                if( !DMXSign[Addr].InfiniteLoop){
                    DMX_Repeat[Addr][0]++;
                    if(DMX_Repeat[Addr][0]==0x10){
                        DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
                        DMX_sumRepeat[Addr]=2;
                        DMXSign[Addr].InfiniteLoop=1;
                    }
                }
            }else{
                DMX_Repeat[Addr][1]=DMX_Repeat[Addr][0];
                DMX_sumRepeat[Addr]=DMX_Repeat[Addr][0]+DMX_Repeat[Addr][1];
                DMX_Repeat[Addr][0]=1;
                DMXSign[Addr].InfiniteLoop=0;
            }
            preRxData[Addr]=RxData[Addr];
            Addr++;
        }
    }
    //DMX didn't get Signal
    if (Timer.MS) {
        DMXPeriodConst++;
        Timer.MS = 0;
        Addr = 0;
        DMXPeriodDimming++;
        if(DMXStepConst==0 && DMXPeriodDimming<DMXPeriod){
            DMXStepConst=DMXStep;
            while (Addr < 4) {
                switch (DMXSign[Addr].SIGN) {
                    case 0b01:
                        DMX_CurrentBright[Addr]=DMX_CurrentBright[Addr]+DMX_SpaceBright[Addr];
                        if(DMX_CurrentBright[Addr]>255){
                            DMX_CurrentBright[Addr]=255;
                        }
                        break;
                    case 0b10:
                        DMX_CurrentBright[Addr]=DMX_CurrentBright[Addr]-DMX_SpaceBright[Addr];
                        if(DMX_CurrentBright[Addr]<0){
                            DMX_CurrentBright[Addr]=0;
                        }
                        break;
                    case 0b00:
                        break;
                }
                rxdata=DMX_CurrentBright[Addr];
                
                CurrentPWM.DC[Addr]=PWM.DC[rxdata]+(PWM.DC[rxdata+1]-PWM.DC[rxdata])*(DMX_CurrentBright[Addr]-rxdata); //interpolation
                *PWMDCHptr[Addr] = CurrentPWM.PWM[Addr].DCH;
                *PWMDCLptr[Addr] = CurrentPWM.PWM[Addr].DCL;
                *PWMLDCONptr[Addr] = 0b10000000;
                
                Addr++;
            }
//        TXREG=CurrentPWM.PWM[3].DCH;
//        TXEN = 1; 
//        TXIE = 1;
        
        }else{
            DMXStepConst--;
        }
        
        // If no data received for 1200ms turn the lights off
//        if (DMX_Flags.RxTimeout == 1) {
//            PWM1DC = PWM2DC = PWM3DC = PWM4DC = 0;
//            PWM1LDCON = PWM2LDCON = PWM3LDCON = PWM4LDCON = 0b10000000;
//            Addr = 0;
//            while (Addr < 4) {
//                DMXSign[Addr].SIGN = 0b00;
//                CurrentPWM.DC[Addr]=0;
//                DMX_TargetBright[Addr]=DMX_CurrentBright[Addr]=0.0;
//                DMX_Repeat[Addr][0]=DMX_Repeat[Addr][1]=1;
//                DMX_sumRepeat[Addr]=2;
//                DMXSign[Addr].InfiniteLoop=1;
//                Addr++;
//            }
//        }
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
//                        DMX_Flags.RxStart = 1; // Indicate a Start
                        DMXPeriod = DMXPeriodConst; // DMX period Record;
                        DMXPeriodConst = 0; // DMX period reset;
                    } else if (RxDat == E120_SC_RDM) { // RDM_StartCode == 0xCC;
                        // Valid Start Received
                        RxState = RX_RDM_READ_SubStartCode;
//                        DMX_Flags.RxStart = 1; // Indicate a Start
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
                if (RxDat == E120_SC_SUB_MESSAGE) // RDM_SubStartCode == 0x01;
                {
                    RxState = RX_RDM_READ_DATA;
                    PackCount = 23;
                    PD_Flag = 0;
                }
                break;

            case RX_RDM_READ_DATA:
                RxDat = RCREG;
                RX_RDM_Data.value[PackCount] = RxDat;

                if (PackCount == 2 && RX_RDM_Data.value[PackCount] > 0 && PD_Flag == 0) {
                    RxState = RX_RDM_PD;
                    PDCount = PD_LEN-1;
                    PackCount--;
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
                PD.u8[PDCount] = RxDat;
                if ((PD_LEN - RX_RDM_Data.PDL) == PDCount) {
                    RxState = RX_RDM_READ_DATA;
                    PD_Flag = 1;
                }else{
                    PDCount--;
                }
                break;
        }
        
        if (RxTimer > DMX_RX_TIMEOUT_MS) {
            DMX_Flags.RxTimeout = 1;
            RxTimer = 0;
        }
    }
}
