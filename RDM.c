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


void RDM_init(void) {
    TimerState = 0;

    Timer.HR_Count = 0;
    Timer.MIN_Count = 0;
    Timer.SEC_Count = 0;
    Timer.flags = 0;

    // Set up the Timer for 1ms Interrupts
    TMR1 = TMR1_LOAD_500US;
    TMR1IF = 0;
    TMR1IE = 1;
    PEIE = 1; // Turn the peripheral interrupt ON
    
    // Set up EUSART options for TX
    TX_PIN = 1; // Set pin (RC4) used high
    TX_TRIS = 0; // Set TX pin (RC4) as output
    TXEN=1;
    SPEN=1;
    SYNC=0;
    TX_PPS=0b1001;  //RS4 connect to TX
//    TXIE = 1; // Only Transmit when ready
    TX_PIN = 1;
    TxByte=&TX_RDM_Data.value[0];
    TX9=TX9D=1;
    
    TxCount = 0;
    
    DMX_Flags.RDMmute=0;
    
//    PD[230]=0xFF;
//    PD[229]=0xCA;
//    PD_Manu=&PD[229];
//    PD[228]=0x12;
//    PD[227]=0x34;
//    PD[226]=0x56;
//    PD[225]=0x78;
//    PD_ID=&PD[225];
    
    //DISCOVERY Response Init
    PDCount=23;
    while(PDCount>17){      //value[23~17]]=0xFE
        DISCOVERY_RDM_Data.value[PDCount]=0xFE;
        PDCount--;
    }
    DISCOVERY_RDM_Data.value[17]=0xAA;
    PDCount=0;
    checkSum=0;
    while(PDCount<6){   //value[16~5]]=Encode UID
        DISCOVERY_RDM_Data.value[16-PDCount*2]=UID[PDCount]|0xAA;
        DISCOVERY_RDM_Data.value[15-PDCount*2]=UID[PDCount]|0x55;
        checkSum+=DISCOVERY_RDM_Data.value[16-PDCount*2];
        checkSum+=DISCOVERY_RDM_Data.value[15-PDCount*2];
        PDCount++;
    }
    TX_RDM_Data.CS=checkSum;
    PDCount=0;  //value[4~1]=CheckSum of EUID
    DISCOVERY_RDM_Data.value[4]=TX_RDM_Data.CSH|0xAA;
    DISCOVERY_RDM_Data.value[3]=TX_RDM_Data.CSH|0x55;
    DISCOVERY_RDM_Data.value[2]=TX_RDM_Data.CSL|0xAA;
    DISCOVERY_RDM_Data.value[1]=TX_RDM_Data.CSL|0x55;
    
    
}

void RDM_rx_loop(void){
    if(DMX_Flags.RDMNew==1){
        DMX_Flags.RDMNew=0;
        DMX_Flags.RDMcheck=0;
        
        //1. check UID
        if(RX_RDM_Data.UID.M==METEOR && RX_RDM_Data.UID.ID==DriverID){
            DMX_Flags.RDMcheck=1;
        }
        
        //2. checksum
        if(DMX_Flags.RDMcheck==1 && RDM_get_checkSum(RX_RDM_Data)==RX_RDM_Data.CS){
            DMX_Flags.RDMcheck=2;
        }
        
//        3. check CC
        if(DMX_Flags.RDMcheck==2){
            TMR1 = TMR_LOAD_MAB;    // Load the MAB time = 0xFFEB  // Load value for MAB      ( 20us)
            TimerState = TIMER_MAB; // Next state is MAB end
            switch(RX_RDM_Data.CC){
                case DISCOVERY_COMMAND:     //0x10
                    RDM_discovery_CC();
                    break;
                case GET_COMMAND:           //0x20
                    break;
                case SET_COMMAND:           //0x30
                    break;
            }
        }
        
//       TX_RDM_Data=RX_RDM_Data;
        //4. check PID
       
        //2. 
        
    }
}

void RDM_discovery_CC(void){
    switch(RX_RDM_Data.PID){
        case DISC_UNIQUE_BRANCH:    //0x0001
            if(!DMX_Flags.RDMmute){
                DMX_Flags.RDMcheck=0;
                PD_Manu=&PD[229];   //PD 0~1
                PD_ID=&PD[225];     //PD 2~5
                if(*PD_Manu<=METEOR ){              // Lower Bound UID
                    DMX_Flags.RDMcheck++;
                    if(*PD_Manu==METEOR && *PD_ID>DriverID){
                        DMX_Flags.RDMcheck=0;
                    }
                }
                
                PD_Manu=&PD[223];   //PD    6~7
                PD_ID=&PD[219];     //PD    8~11
                if(*PD_Manu>=METEOR){           // Upper Bound UID
                    DMX_Flags.RDMcheck++;
                    if(*PD_Manu==METEOR && *PD_ID<DriverID){
                        DMX_Flags.RDMcheck=0;
                    }
                }
                if(DMX_Flags.RDMcheck==2){  //if is in range...
                    RDM_tx_TimerBreak();        //180ua Break...
                    TxState=TX_DISCOVERY_START;    
                }
            }
            break;
        case DISC_MUTE:             //0x0002
                DMX_Flags.RDMmute=1;
            break;
        case DISC_UN_MUTE:          //0x0003
                DMX_Flags.RDMmute=0;
            break;
        default:
            break;
    }
}




void RDM_tx_interrupt(void) {
    if (TXIE && TXIF) {
//        RC3=~RC3;
        
//        TXIE=0;
        
//        if (TxCount < TX_SIZE) {
//            TXREG = RxData[0];
//            TxByte++; // Point to next byte
//            TxCount++; // Count the addresses
//        }
//        else{
//            TXREG = TxStart;//TxStart == 0x00 == DMX_StartCode
//                        TxByte = &TxData[0]; // Point to First byte
//            TxCount = 0; // Clear the address counter
//        }
        
        switch (TxState) {
            case TX_MARK_BEFORE_BREAK:

                break;

//            case TX_MAB:
//                break;
            case TX_START:
                TXREG = RDM_SubStartCode;//0x01
                TX_SIZE=24;
                TxByte = &TX_RDM_Data.value[23]; // Point to Last byte
                TxCount = 0;
                TxState = TX_DATA;
                break;
                
            case TX_DISCOVERY_START: // Send start and set up for data stream
                TXREG = 0xFE;//0xFE for slot 1
                TX_SIZE=23;
                TxByte= &DISCOVERY_RDM_Data.value[23];   // Point to Last byte
                TxCount = 0; // Clear the address counter
                TxState = TX_DATA;
                break;

            case TX_DATA:                 // Send the data
                if (TxCount < TX_SIZE) {  //TxCount<23 or 24
                    TXREG = *TxByte;
                    TxByte--; // Point to next byte
                    TxCount++; // Count the addresses
                }else{
                    TX_PIN = 1; // Ensure PIN = RC4 will be high
                    TXEN = 0; // Disable the EUSART's control of the TX pin
    //                Timer.BREAK = 1; // Indticate a BREAK to the timer interrupt
                    TXIE = 0; // Disable the EUSART Interrupt 
                    TxState = TX_DISCOVERY_START; 
                    
                    TXREG = 0; // Put one more byte in the buffer to force next interrupt
                    TxState = TX_MARK_BEFORE_BREAK; // Next interrupt go to BREAK
//                    DMX_Flags.TxDone = 1;
                }
                
                break;
        }
    }
}

uint16_t RDM_get_checkSum(RDM_Data Data){
    checkSum=0x00CD;   // 0xCC+ 0x01
    PackCount=0;
    while(PackCount<22){
        checkSum+=Data.value[PackCount];
        PackCount++;
    }
    PackCount=0;
    while(PackCount<Data.PDL){
        checkSum+=PD[PackCount];
        PackCount++;
    }
    return checkSum;
}

void RDM_tx_TimerBreak(){
    TMR1 = TMR_LOAD_BREAK; //=0xFF4B  Load Value for BREAK    (180us)
    TX_PIN = 0;//?SPACE? for BREAK
//    TimerState = TIMER_BREAK;
//    Timer.BREAK = 0;
    TXREG = RDM_StartCode;  //0xCC
//    
//    TX_RDM_Data.UID.M=RX_RDM_Data.CUID.M;
//    TX_RDM_Data.UID.ID=RX_RDM_Data.CUID.ID;
//    TX_RDM_Data.CUID.M=RX_RDM_Data.UID.M;
//    TX_RDM_Data.CUID.ID=RX_RDM_Data.UID.ID;
//    
//    TX_RDM_Data.CC=TX_RDM_Data.CC+1;
//    
//    TX_RDM_Data.ML=24+TX_RDM_Data.PDL;
//    uint16_tmp=RDM_get_checkSum(TX_RDM_Data);
//    TX_RDM_Data.value[22]=(uint16_tmp>>8);
//    TX_RDM_Data.value[23]=(uint16_tmp&0x00FF);
}

