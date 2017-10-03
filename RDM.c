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
#include "rdm_define.h"
#include "PFM.h"

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
    TXEN = 1;
    SPEN = 1;
    SYNC = 0;
    TX_PPS = 0b1001; //RS4 connect to TX
    //    TXIE = 1; // Only Transmit when ready
    TxByte = &TX_RDM_Data.value[0];
    TX9 = TX9D = 1;

//    TxCount = 0;

    DMX_Flags.RDMmute = 0;
    DMX_Flags.RDM_Identify_Device = 0;
//    DataPtr = &PD[0];
    //    PD[230]=0xFF;
    //    PD[229]=0xCA;
    //    PD_Manu=&PD[229];
    //    PD[228]=0x12;
    //    PD[227]=0x34;
    //    PD[226]=0x56;
    //    PD[225]=0x78;
    //    PD_ID=&PD[225];
//    PFM_Write(Flash_DMXAddress,0x0021);
    
//    DMX_Address=PFM_Read(Flash_DMXAddress);
    
//    if(DMX_Address==0 || DMX_Address>512){
        DMX_Address=1;
//    }
    
    
    //DISCOVERY Response Init
    PDCount = TX_SIZE;
//    DISCOVERY_RDM_Data.value[PDCount]=0xCC;
//    PDCount--;
    while (PDCount >= 17) { //value[23~17]]=0xFE
        DISCOVERY_RDM_Data[PDCount] = 0xFE;
        PDCount--;
    }
    DISCOVERY_RDM_Data[16] = 0xAA;
    PDCount = 0;
    checkSum = 0;
    while (PDCount < 6) { //value[15~5]]=Encode UID
        DISCOVERY_RDM_Data[15 - PDCount * 2] = UID[PDCount] | 0xAA;
        DISCOVERY_RDM_Data[14 - PDCount * 2] = UID[PDCount] | 0x55;
        checkSum += DISCOVERY_RDM_Data[15 - PDCount * 2];
        checkSum += DISCOVERY_RDM_Data[14 - PDCount * 2];
        PDCount++;
    }
    TX_RDM_Data.CS = checkSum;
//    PDCount = 0; //value[3~0]=CheckSum of EUID
    DISCOVERY_RDM_Data[3] = TX_RDM_Data.CSH | 0xAA;
    DISCOVERY_RDM_Data[2] = TX_RDM_Data.CSH | 0x55;
    DISCOVERY_RDM_Data[1] = TX_RDM_Data.CSL | 0xAA;
    DISCOVERY_RDM_Data[0] = TX_RDM_Data.CSL | 0x55;
    
    PDCount = 16;
    while (PDCount >= 11) { //value[16~11]]=Source UID(Meteor UID)
        TX_RDM_Data.value[PDCount] = UID[16-PDCount];
        PDCount--;
    }
//    if(DMX_Address=PFM_Read(Flash_DMXAddress)>0){
//        
//    }
}

void RDM_rx_loop(void) {
    if (DMX_Flags.RDMNew == 1) {
        DMX_Flags.RDMNew = 0;
        DMX_Flags.RDMcheckUID_flag = 0;
        //1. check UID
        if ((RX_RDM_Data.DUID.M == METEOR || RX_RDM_Data.DUID.M == 0xFFFF)&& (RX_RDM_Data.DUID.ID == DriverID || RX_RDM_Data.DUID.ID == 0xFFFFFFFF)) {
            DMX_Flags.RDMcheckUID_flag = 1;
        }
        //        2. checksum
        if (DMX_Flags.RDMcheckUID_flag == 1 && RDM_get_checkSum(RX_RDM_Data,PD_LEN) == RX_RDM_Data.CS) {
            DMX_Flags.RDMcheckUID_flag = 2;
        }
        //        3. check CC and do
        if (DMX_Flags.RDMcheckUID_flag == 2) {
            switch (RX_RDM_Data.CC) {
                case E120_DISCOVERY_COMMAND: //0x10
                    RDM_discovery_CC();
                    break;
                case E120_GET_COMMAND: //0x20
                    RDM_GET_CC();
                    break;
                case E120_SET_COMMAND: //0x30
                    RDM_SET_CC();
                    break;
            }
        }
    }
}

void RDM_tx_interrupt(void) {
    if (TXIE && TXIF) {
        
//        TXREG=1;
//        TXEN = 0; // Disable the EUSART's control of the TX pin
//        TXIE = 0; // Disable the EUSART Interrupt 
        
        switch (TxState) {
            case TX_START:
                if(TX_PD_Flag==1){
                    TX_PD_Flag=0;
                    TXREG = E120_SC_RDM;
                }else{
                    TXREG = E120_SC_SUB_MESSAGE;//0x01
                    TxByte = &TX_RDM_Data.value[23]; // Point to Last byte
                    TxCount = 0;
                    TxState = TX_DATA;
                }
                break;
            case TX_SART_DISCOVERY:
                TxState = TX_DISCOVERY;
                break;
            case TX_DISCOVERY: // Send start and set up for data stream
                if (TxCount <= DiscoveryLength) {
                    TXREG = *TxByte;
                    TxByte--; // Point to next byte
                    TxCount++; // Count the addresses
                }else{
//                    RCIE = 1; //Enable RC interrupt
                    TX_PIN = 1; // Ensure PIN = RC4 will be high
                    TXEN = 0; // Disable the EUSART's control of the TX pin
                    TXIE = 0; // Disable the EUSART Interrupt 
                    RXTX_SWITCH_PIN=0;  //Set switch pin to RX mode
                }
                break;
            case TX_DATA:                 // Send the data
                if (TxCount <= TX_SIZE) {
//                    TX_RDM_Data.value[1]=0x11;
                    TXREG = *TxByte;
                    TxByte--; // Point to next byte
                    TxCount++; // Count the addresses
                    if(TxCount == 22 & TX_RDM_Data.PDL > 0 & TX_PD_Flag == 0){
                        TxState = TX_RDM_PD;
                        TX_PDCount = TX_PD_LEN-1;
                    }
                }else{
//                    RCIE = 1; //Enable RC interrupt
                    TX_PIN = 1; // Ensure PIN = RC4 will be high
                    TXEN = 0; // Disable the EUSART's control of the TX pin
                    TXIE = 0; // Disable the EUSART Interrupt 
                    RXTX_SWITCH_PIN=0;  //Set switch pin to RX mode
                }
                break;
            case TX_RDM_PD:
                TXREG = PD.u8[TX_PDCount];
                if (TX_PDCount == TX_PD_LEN-TX_RDM_Data.PDL) {
                    TxState = TX_DATA;
                    TX_PD_Flag = 1;
                }else{
                    TX_PDCount--;
                }
                break;
        }
    }
}

void RDM_tx_TimerBreak() {
    TMR1 = TMR_LOAD_BREAK; //=0xFF4B  Load Value for BREAK    (180us)
    TimerState = TIMER_BREAK;
    TX_PIN = 0; //set low ?SPACE? for BREAK
    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
    TX_PD_Flag=1;//check you have sent Parameter Data or not.
//    RCIE = 0; //Enable RC interrupt
}

//void RDM_disc_tx_TimerBreak(){
//    TMR1 = TMR_DISC_MARK; //=0xFFFB  Load Value for MARK    (4us)
//    TimerState = TIMER_DISC_MARK;
//    TX_PIN = 1; //set high ?SPACE? for MARK
//    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//    RCIE = 0; //Enable RC interrupt
//}

//
//
//uint16_t RDM_set_checkSum(RDM_Data Data) {
//    checkSum = 0x00CD; // 0xCC+ 0x01
//    PackCount = 23;
//    while (PackCount > 1) {
//        checkSum += Data.value[PackCount];
//        PackCount--;
//    }
//    PackCount = PD_LEN-1;
//    PDCount = PD_LEN - Data.PDL;
//    while (PackCount >= PDCount) {
//        checkSum += PD[PackCount];
//        PackCount--;
//    }
//    return checkSum;
//}

//void PWM_Level_interrupt() {
//    if (TXIE && TXIF) {
//        TXIE = 0;
//    }
//}