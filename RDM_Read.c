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
            TX_RDM_Data.PDL=1;//Control Field (2 slot)
            PD.u8[TX_PD_LEN-1] =DMX_Flags.RDMidentify;
            break;
        case E120_DEVICE_INFO:
            TX_RDM_Data.PDL=0x13;//Control Field (19 slot)
            PD.u16[TX_PD_u16-1]=E120_PROTOCOL_VERSION;          //RDM Protocol Version  //uint16_t 
            PD.u16[TX_PD_u16-2]=E120_DEVICE_MODEL_DESCRIPTION;  //Device Model ID
            PD.u16[TX_PD_u16-3]=E120_PRODUCT_DETAIL_LED;        //Product Category
            PD_ID=&PD.u8[TX_PD_LEN-10]; //SoftWareVersion  //uint32_t
            *PD_ID=1;
            PD.u16[TX_PD_u16-6]=0x04;//FootPrint
            PD.u16[TX_PD_u16-7]=0x01;//Personality
            PD.u16[TX_PD_u16-8]=DMX_Address;//StartAddress
            PD.u16[TX_PD_u16-9]=0;//Sub-Device Count
            PD.u8[TX_PD_LEN-19]=0; //Sensor Count
            break;
        case E120_DMX_START_ADDRESS:
            TX_RDM_Data.PDL=2;//Control Field (2 slot)
//            PD_Manu=&PD.u16[TX_PD_LEN-2]; //StartAddress
            PD.u16[TX_PD_u16-1]=DMX_Address;
            break;
    }
    RDM_TXSTART();
}

void RDM_SET_CC(void){
    switch (RX_RDM_Data.PID){
        case E120_DMX_START_ADDRESS:
//            PD_Manu=&PD[PD_LEN-2]; 
            DMX_Address=PD.u16[PD_u16-1]; //StartAddress
//            PFM_Write(Flash_DMXAddress,DMX_Address);
            TX_RDM_Data.PDL=0;//Control Field (2 slot)
//            DMX_Address=PFM_Read(Flash_DMXAddress);
            break;
    }
    RDM_TXSTART();
}

void RDM_discovery_CC(void){
    switch (RX_RDM_Data.PID) { //RX_RDM_Data.PID
        case E120_DISC_UNIQUE_BRANCH: //0x0001
            if (!DMX_Flags.RDMmute) {
                DMX_Flags.RDMcheck = 1;
                PD_Manu = &PD.u8[PD1]; //PD 0~1
                PD_ID = &PD.u8[PD5]; //PD 2~5
                if (*PD_Manu <= METEOR) { // Lower Bound UID
                    DMX_Flags.RDMcheck++;
                    if (*PD_Manu == METEOR && *PD_ID > DriverID) {
                        DMX_Flags.RDMcheck = 0;
                    }
                }
                PD_Manu = &PD.u8[PD7]; //PD    6~7
                PD_ID = &PD.u8[PD11]; //PD    8~11
                if (*PD_Manu >= METEOR) { // Upper Bound UID
                    DMX_Flags.RDMcheck++;
                    if (*PD_Manu == METEOR && *PD_ID < DriverID) {
                        DMX_Flags.RDMcheck = 0;
                    }
                }
                if (DMX_Flags.RDMcheck == 3) { //if is in range...
                    TxState = TX_DISCOVERY;
                    TXREG = 0xFE;
                    TxByte= &DISCOVERY_RDM_Data.value[23];   // Point to Last byte
                    TxCount = 0; // Clear the address counter
                    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
                    RCIE = 0; //Enable RC interrupt
                    TXEN = 1; // Re-enable EUSART control of pin    
                    TXIE = 1; // Re-Enable EUSART Interrupt     , it will send RDM_StartCode (0xCC)
                }
            }
            break;

        case E120_DISC_MUTE: //0x0002
            DMX_Flags.RDMmute = 1;
            TX_RDM_Data.PDL=2;//Control Field (2 slot)
            PD.u16[TX_PD_u16-1] = 0;
            RDM_TXSTART();
            break;
        case E120_DISC_UN_MUTE: //0x0003
            DMX_Flags.RDMmute = 0;
            TX_RDM_Data.PDL=2;//Control Field (2 slot)
            PD.u16[TX_PD_u16-1] = 0;
            RDM_TXSTART();
            break;
        default:
            break;
    }
}

void RDM_TXSTART(void){
    TXREG = E120_SC_RDM;
    TX_RDM_Response_Set();
    TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_PD_LEN);
    TxState = TX_START;
    RDM_tx_TimerBreak(); //180ua Break... and Start to Response Discovery Signal
}

void TX_RDM_Response_Set(){
    TX_RDM_Data.DUID.ID=RX_RDM_Data.SUID.ID;
    TX_RDM_Data.DUID.M=RX_RDM_Data.SUID.M;
    TX_RDM_Data.TN=RX_RDM_Data.TN;
    TX_RDM_Data.CC=RX_RDM_Data.CC+1;
    TX_RDM_Data.PID=RX_RDM_Data.PID;
    TX_RDM_Data.ML=24+TX_RDM_Data.PDL;  
    TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
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
        checkSum += PD.u8[PDCount];
        PDCount++;
    }
    return checkSum;
}



