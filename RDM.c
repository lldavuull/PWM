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
    
    PEIE = 1; // Turn the peripheral interrupt ON
    // Set up EUSART options for TX
    TX_TRIS = 0; // Set TX pin (RC4) as output
    TX_PIN = 1; // Set pin (RC4) used high
//    TXEN = 1;
    SPEN = 1;
    SYNC = 0;
    TX_PPS = 0b1001; //RS4 connect to TX
    //    TXIE = 1; // Only Transmit when ready
    BytePtr = &TX_RDM_Data.value[0];
    TX9 = TX9D = 1;

    RXTX_SWITCH_PIN=0;
    DMX_Flags.RDMmute = 0;
    DMX_Flags.RDM_Identify_Device = 0;
    
    //DISCOVERY Response Init
    PDCount = DiscoveryLength;
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
    DMX_Address=PFM_Read(Flash_DMXAddress);
    if(DMX_Address==0x3fff){
        PFM_Write(Flash_DMXAddress,0x0001);
    }
    DMX_Address=PFM_Read(Flash_DMXAddress);
    
    PERSONALITY=PFM_Read(Flash_Personality);
    if(PERSONALITY==0xff){
        PFM_Write(Flash_Personality,0x0004);
    }
    PERSONALITY=PFM_Read(Flash_Personality);
    
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
            PD_init();
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
        switch (TxState) {
            case TX_START:
                if(TX_PD_Flag==1){
                    TX_PD_Flag=0;
                    TXREG = E120_SC_RDM;
                }else{
                    TXREG = E120_SC_SUB_MESSAGE;//0x01
                    BytePtr = &TX_RDM_Data.value[23]; // Point to Last byte
                    TxCount = 0;
                    TxState = TX_DATA;
                }
                break;
            case TX_SART_DISCOVERY:
                TxState = TX_DISCOVERY;
                break;
            case TX_DISCOVERY: // Send start and set up for data stream
                if (TxCount <= DiscoveryLength) {
                    TxCount++; // Count the addresses
                    TXREG = *BytePtr;
                    BytePtr--; // Point to next byte
                }else{
//                    TXEN = 0; // Disable the EUSART's control of the TX pin
                    TXIE = 0; // Disable the EUSART Interrupt
                    RCIE = 1; //Enable RC interrupt
                    TMR1 = TMR_LOAD_MAS; // Load the MAB time = 0xFFFA  // Load value for MBB      ( 4us)
                    TimerState = TIMER_MAS; // Next state is MAB end
//                    TX_PIN = 1; // Set pin high for MAB
//                    TX_PD_Flag=0;
                }
                break;
            case TX_DATA:                 // Send the data
                if (TxCount <= TX_SIZE) {
//                    TX_RDM_Data.value[1]=0x11;
                    TXREG = *BytePtr;
                    BytePtr--; // Point to next byte
                    TxCount++; // Count the addresses
                    if(TxCount == 22 & TX_RDM_Data.PDL > 0 & TX_PD_Flag == 0){
                        TxState = TX_RDM_PD;
                        TX_PDCount = TX_PD_LEN-1;
                    }
                }else{
                    RCIE = 1; //Enable RC interrupt
//                    TXEN = 0; // Disable the EUSART's control of the TX pin
                    TXIE = 0; // Disable the EUSART Interrupt 
//                    TMR1 == TX_TIMER_MBB; // Next state is MAB end
//                    RXTX_SWITCH_PIN=0;  //Set switch pin to RX mode
//                    TX_PIN = 1; // Ensure PIN = RC4 will be high
                    TMR1 = TMR_LOAD_MAS; // Load the MAB time = 0xFFFA  // Load value for MBB      ( 4us)
                    TimerState = TIMER_MAS; // Next state is MAB end
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
    TMR1 = TMR_LOAD_RDM_MBB; //=0xFF4B  Load Value for BREAK    (180us)
    TimerState = TIMER_RDM_MBB;
    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
    TX_PD_Flag=1;//check you have sent Parameter Data or not.
//    RCIE = 0; //Enable RC interrupt
}