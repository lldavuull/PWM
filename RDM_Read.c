/*
 * File:   RDM_Read.c
 * Author: METEOR
 *
 * Created on 2017?8?1?, ?? 8:35
 */


#include <xc.h>
#include "RDM.h"
#include "rdm_define.h"
#include "DMX.h"
#include "PFM.h"

void RDM_GET_CC(void){
    switch (RX_RDM_Data.PID){
        case E120_IDENTIFY_DEVICE:  //0x1000
            TX_RDM_Response_Set();
            TX_RDM_Data.ML=26;  // include Control Field (2 slot)  24+2=26 slot
            TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
            TX_RDM_Data.PDL=1;//Control Field (2 slot)
            PD[TX_PD_LEN-1] =DMX_Flags.RDMidentify;
            break;
            
        case E120_DEVICE_INFO:
            TX_RDM_Data.PDL=0x13;//Control Field (19 slot)
            TX_RDM_Response_Set();
            TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
            PD_Manu=&PD[TX_PD_LEN-2]; //RDM Protocol Version PD[0~1]
            *PD_Manu=E120_PROTOCOL_VERSION;
            PD_Manu=&PD[TX_PD_LEN-4]; //Device Model ID PD[2~3]
            *PD_Manu=E120_DEVICE_MODEL_DESCRIPTION;
            PD_Manu=&PD[TX_PD_LEN-6]; //Product Category
            *PD_Manu=E120_PRODUCT_DETAIL_LED;
            PD_ID=&PD[TX_PD_LEN-10]; //SoftWareVersion PD[6~9]
            *PD_ID=1;
            PD_Manu=&PD[TX_PD_LEN-12]; //FootPrint  PD[10~11]
            *PD_Manu=0x04;
            PD_Manu=&PD[TX_PD_LEN-14]; //Personality
            *PD_Manu=0x01;
            PD_Manu=&PD[TX_PD_LEN-16]; //StartAddress
            *PD_Manu=DMX_Address;
            PD_Manu=&PD[TX_PD_LEN-18]; //Sub-Device Count
            *PD_Manu=0;
            PD[TX_PD_LEN-19]=0; //Sensor Count
//            TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_RDM_Data.PDL);
//            TxState = TX_START;
//            RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
            break;
        case E120_DMX_START_ADDRESS:
            PD_Manu=&PD[TX_PD_LEN-2]; //StartAddress
            *PD_Manu=DMX_Address;
            TX_RDM_Response_Set();
            TX_RDM_Data.ML=26;  // include Control Field (2 slot)  24+2=26 slot
            TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
            TX_RDM_Data.PDL=2;//Control Field (2 slot)
            break;
    }
    TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_PD_LEN);
    TxState = TX_START;
    TXREG = E120_SC_RDM;
    RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
}


void RDM_SET_CC(void){
    switch (RX_RDM_Data.PID){
        case E120_DMX_START_ADDRESS:
            PD_Manu=&PD[PD_LEN-2]; //StartAddress
            DMX_Address=*PD_Manu;
            TX_RDM_Response_Set();
            TX_RDM_Data.ML=24;  // include Control Field (2 slot)  24+2=26 slot
            TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
            TX_RDM_Data.PDL=0;//Control Field (2 slot)
            break;
    }
    TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_PD_LEN);
    TxState = TX_START;
    TXREG = E120_SC_RDM;
    RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
}

void RDM_discovery_CC(void){
    switch (RX_RDM_Data.PID) { //RX_RDM_Data.PID
        case E120_DISC_UNIQUE_BRANCH: //0x0001
            if (!DMX_Flags.RDMmute) {
                DMX_Flags.RDMcheck = 1;
                PD_Manu = &PD[PD1]; //PD 0~1
                PD_ID = &PD[PD5]; //PD 2~5
                if (*PD_Manu <= METEOR) { // Lower Bound UID
                    DMX_Flags.RDMcheck++;
                    if (*PD_Manu == METEOR && *PD_ID > DriverID) {
                        DMX_Flags.RDMcheck = 0;
                    }
                }
                PD_Manu = &PD[PD7]; //PD    6~7
                PD_ID = &PD[PD11]; //PD    8~11
                if (*PD_Manu >= METEOR) { // Upper Bound UID
                    DMX_Flags.RDMcheck++;
                    if (*PD_Manu == METEOR && *PD_ID < DriverID) {
                        DMX_Flags.RDMcheck = 0;
                    }
                }
                if (DMX_Flags.RDMcheck == 3) { //if is in range...
                    TxState = TX_SART_DISCOVERY;
                    TXREG = 0xCC;
                    TxByte= &DISCOVERY_RDM_Data.value[23];   // Point to Last byte
                    TxCount = 0; // Clear the address counter
                    RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
                }
            }
            break;

        case E120_DISC_MUTE: //0x0002
            DMX_Flags.RDMmute = 1;
            TX_RDM_Response_Set();
            TX_RDM_Data.ML=26;  // include Control Field (2 slot)  24+2=26 slot
            TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
            TX_RDM_Data.PDL=2;//Control Field (2 slot)
            PD[TX_PD_LEN-1] = PD[TX_PD_LEN-2] = 0;
            TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_PD_LEN);
            TxState = TX_START;
            TXREG = E120_SC_RDM;
            RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
            break;
        case E120_DISC_UN_MUTE: //0x0003
            DMX_Flags.RDMmute = 0;
            break;
        default:
            break;
    }
}


void TX_RDM_Response_Set(){
    TX_RDM_Data.DUID.ID=RX_RDM_Data.SUID.ID;
    TX_RDM_Data.DUID.M=RX_RDM_Data.SUID.M;
    TX_RDM_Data.TN=RX_RDM_Data.TN;
    TX_RDM_Data.CC=RX_RDM_Data.CC+1;
    TX_RDM_Data.PID=RX_RDM_Data.PID;
}


uint16_t RDM_get_checkSum(RDM_Data Data, char len) {
    checkSum = 0x00CD; // 0xCC+ 0x01
    PackCount = 23;
    while (PackCount > 1) {
        checkSum += Data.value[PackCount];
        PackCount--;
    }
    PackCount = len-1;
    PDCount = len - Data.PDL;
    while (PackCount >= PDCount) {
        checkSum += PD[PDCount];
        PDCount++;
    }
    return checkSum;
}



