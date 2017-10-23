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
    //UID Setting
    UID.MANC=0x08BA;
    UID.ID=0x12345678;
    //DISCOVERY Response Init
    tmp8 = DiscoveryLength;
//    DISCOVERY_RDM_Data.value[PDCount]=0xCC;
//    PDCount--;
    while (tmp8 >= 17) { //value[23~17]]=0xFE
        DISCOVERY_RDM_Data[tmp8] = 0xFE;
        tmp8--;
    }
    DISCOVERY_RDM_Data[16] = 0xAA;
    tmp8 = 0;
    checkSum = 0;
    while (tmp8 < 6) { //value[15~5]]=Encode UID
        DISCOVERY_RDM_Data[15 - tmp8 * 2] = UID.data[5-tmp8] | 0xAA;
        DISCOVERY_RDM_Data[14 - tmp8 * 2] = UID.data[5-tmp8] | 0x55;
        checkSum += DISCOVERY_RDM_Data[15 - tmp8 * 2];
        checkSum += DISCOVERY_RDM_Data[14 - tmp8 * 2];
        tmp8++;
    }
    TX_RDM_Data.CS = checkSum;
//    PDCount = 0; //value[3~0]=CheckSum of EUID
    DISCOVERY_RDM_Data[3] = TX_RDM_Data.CSH | 0xAA;
    DISCOVERY_RDM_Data[2] = TX_RDM_Data.CSH | 0x55;
    DISCOVERY_RDM_Data[1] = TX_RDM_Data.CSL | 0xAA;
    DISCOVERY_RDM_Data[0] = TX_RDM_Data.CSL | 0x55;
    tmp8 = 16;
    while (tmp8 >= 11) { //value[16~11]]=Source UID(Meteor UID)
        TX_RDM_Data.value[tmp8] = UID.data[tmp8-11];
        tmp8--;
    }
    
    //get Flash_DMX_ADDRESS
    DMX_Address=PFM_Read(Flash_DMX_ADDRESS);
    if(DMX_Address==0x3fff){
//        PFM_Erase(Flash_DMX_ADDRESS);
        PFM_Write(Flash_DMX_ADDRESS,0x0001);
        DMX_Address=PFM_Read(Flash_DMX_ADDRESS);
    }
    
    //get PERSONALITY
    tmp16=PFM_Read(Flash_PERSONALITY);
    if(tmp16==0x3fff){
        PFM_Erase(Flash_PERSONALITY);
        PFM_Write(Flash_PERSONALITY,0x0004);
    }
    PERSONALITY=PFM_Read(Flash_PERSONALITY);
    FOOTPRINT=Get_FootPrint(PERSONALITY);
    
    //get DEVICE_LABEL
    tmp16=PFM_Read(Flash_DEVICE_LABEL);
//    PFM_write_String(Flash_DEVICE_LABEL,&DEVICE_LABEL[0],DEVICE_LABEL_Size);
//    DEVICE_LABEL_Size=PFM_Read_String(Flash_DEVICE_LABEL,&DEVICE_LABEL[0]);
    if(tmp16==0x3fff){
//        PFM_write_String(Flash_DEVICE_LABEL,&DEVICE_LABEL[0],DEVICE_LABEL_Size);
    }else{
        DEVICE_LABEL_SIZE=PFM_Read_String(Flash_DEVICE_LABEL,&DEVICE_LABEL[0]);
    }
}

void RDM_rx_loop(void) {
    if (DMX_Flags.RDMNew == 1) {
        DMX_Flags.RDMNew = 0;
        DMX_Flags.RDMcheckUID_flag = 0;
        //1. check UID
        if ((RX_RDM_Data.DUID.M == UID.MANC || RX_RDM_Data.DUID.M == 0xFFFF)&& (RX_RDM_Data.DUID.ID == UID.ID || RX_RDM_Data.DUID.ID == 0xFFFFFFFF)) {
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
                if (TxCount < TX_SIZE) {
//                    TX_RDM_Data.value[1]=0x11;
                    TXREG = *BytePtr;
                    BytePtr--; // Point to next byte
                    TxCount++; // Count the addresses
                    if(TxCount == 22 & TX_RDM_Data.PDL > 0 & TX_PD_Flag == 0){
                        TxState = TX_RDM_PD;
                        TX_PDCount = TX_PD_LEN-1;
                    }
                }else{
//                    TxState = TX_MAS;
                    RCIE = 1; //Enable RC interrupt
//                    TXEN = 0; // Disable the EUSART's control of the TX pin
                    TXIE = 0; // Disable the EUSART Interrupt 
                    TMR1 = TMR_LOAD_MAS; // Load the MAB time = 0xFFFA  // Load value for MBB      ( 4us)
                    TimerState = TIMER_MAS; // Next state is MAB end
                }
                break;
//            case TX_MAS:                 // Send the data
//                TXIE = 0; // Disable the EUSART Interrupt 
//                RXTX_SWITCH_PIN=0;
//                break;
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
//    TMR1 = TMR_LOAD_RDM_MBB; // Load value for MBB      ( 4us)
//    TimerState = TIMER_RDM_MBB;
//    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//    TX_PD_Flag=1;//check you have sent Parameter Data or not.
    
    
    TXEN = 0;
    TX_PD_Flag=1;//check you have sent Parameter Data or not.
    TMR1 = TMR_LOAD_RDM_BREAK; // Load Value for BREAK    (180us)
    TimerState = TIMER_RDM_BREAK; // Next state is MAB end
    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
}

uint8_t Get_FootPrint(uint8_t Input_Personality){
    return PERSONALITY_DEFINITIONS[Input_Personality-1].footprint;
}