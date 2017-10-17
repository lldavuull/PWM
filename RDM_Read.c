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
#include "Timer.h"

void RDM_GET_CC(void){
    switch (RX_RDM_Data.PID){
        case E120_IDENTIFY_DEVICE:  //0x1000
//            TX_RDM_Data.PDL=1;//Control Field (2 slot)
//            PD.u8[TX_PD_LEN-1] =DMX_Flags.RDM_Identify_Device; // 1= True, 0=False
            PD_write_u8(DMX_Flags.RDM_Identify_Device);// 1= True, 0=False
            break;
        case E120_DEVICE_INFO:
//            TX_RDM_Data.PDL=0x13;//Control Field (19 slot)
//            PD.u16[TX_PD_u16-1]=E120_PROTOCOL_VERSION;          //RDM Protocol Version  //uint16_t
//            PD.u16[TX_PD_u16-2]=E120_DEVICE_MODEL_DESCRIPTION;  //Device Model ID
//            PD.u16[TX_PD_u16-3]=E120_PRODUCT_DETAIL_LED;        //Product Category
//            PD_ID=&PD.u8[TX_PD_LEN-10]; //SoftWareVersion  //uint32_t
//            *PD_ID=0x1003;
//            PD.u16[TX_PD_u16-6]=0x04;//FootPrint
//            PD.u16[TX_PD_u16-7]=0x01;//Personality
//            PD.u16[TX_PD_u16-8]=DMX_Address;//StartAddress
//            PD.u16[TX_PD_u16-9]=0;//Sub-Device Count
//            PD.u8[TX_PD_LEN-19]=0; //Sensor Count
            
            PD_write_u16(E120_PROTOCOL_VERSION);//RDM Protocol Version  //uint16_t
            PD_write_u16(E120_DEVICE_MODEL_DESCRIPTION);//Device Model ID
            PD_write_u16(E120_PRODUCT_DETAIL_LED);//Product Category
            PD_write_u32(0x20171012);//SoftWareVersion (uint32)
            PD_write_u16(0x0004);//FootPrint
            PD_write_u8(PERSONALITY);//Personality
            PD_write_u8(PERSONALITY_SIZE);//Personality Size
            PD_write_u16(DMX_Address);//StartAddress
            PD_write_u16(0);//Sub-Device Count
            PD_write_u8(0);//Sensor Count
            break;
        case E120_DMX_START_ADDRESS:
//            TX_RDM_Data.PDL=2;//Control Field (2 slot)
//            PD.u16[TX_PD_u16-1]=DMX_Address;
            PD_write_u16(DMX_Address);
            break;
        case E120_SOFTWARE_VERSION_LABEL:
//            TX_RDM_Data.PDL=SOFTWARE_VERSION_LABEL_Size;//Control Field (2 slot)
//            *BytePtr=&SOFTWARE_VERSION_LABEL[0];
            while(PackCount<SOFTWARE_VERSION_LABEL_Size){
                PackCount++;
                PD_write_u8(SOFTWARE_VERSION_LABEL[PackCount]);
            }
            break;
        case E120_SUPPORTED_PARAMETERS:
            PDCount=sizeof(PID_DEFINITIONS);
            while(PackCount<PDCount){
                PD_write_u16(PID_DEFINITIONS[PackCount]);
                PackCount++;
            }
            break;
//        case E137_1_IDENTIFY_MODE:
//            PD_write_u8(IDENTIFY_MODE); //Intensity of IDENTIFY
//            break;
        case E120_DMX_PERSONALITY:
            PD_write_u8(PERSONALITY);//Current Personality
            PD_write_u8(PERSONALITY_SIZE);// Total of Personalities
            break;
        case E120_DMX_PERSONALITY_DESCRIPTION:
            BytePtr=PERSONALITY_DEFINITIONS[PERSONALITY-1].description;
            PDCount=PERSONALITY_DEFINITIONS[PERSONALITY-1].descriptionsize;
            while(PackCount<PDCount){
                PD_write_u8(*BytePtr);//Write PersonalityDesciption
                PackCount++;
                BytePtr++;
            }
            break;
    }
    RDM_TXSTART();
}

void RDM_SET_CC(void){
    switch (RX_RDM_Data.PID){
        case E120_DMX_START_ADDRESS:
//            PD_Manu=&PD[PD_LEN-2];
            DMX_Address=*PD_Read_u16ptr();//StartAddress
//            DMX_Address=PD.u16[PD_u16-1]; //StartAddress
            PFM_Write(Flash_DMXAddress,DMX_Address);
//            TX_RDM_Data.PDL=0;//Control Field (2 slot)
//            DMX_Address=PFM_Read(Flash_DMXAddress);
            break;
        case E120_IDENTIFY_DEVICE:  //0x1000
            RDM_Identify_Switch();
            break;
        case E137_1_IDENTIFY_MODE:
            IDENTIFY_MODE=*PD_Read_u8ptr();
            break;
        case E120_DMX_PERSONALITY:
            PERSONALITY=*PD_Read_u8ptr();
            PFM_Write(Flash_Personality,PERSONALITY);
            break;
    }
    RDM_TXSTART();
}

void RDM_discovery_CC(void){
    switch (RX_RDM_Data.PID) { //RX_RDM_Data.PID
        case E120_DISC_UNIQUE_BRANCH: //0x0001
            DMX_Flags.RDMcheckUID_flag = 1;
            if (!DMX_Flags.RDMmute) {
                DMX_Flags.RDMcheckUID_flag = 1;
                PD_Manu_ptr =PD_Read_u16ptr(); //PD 0~1
                PD_ID_ptr = PD_Read_u32ptr(); //PD 2~5
                
                if (*PD_Manu_ptr < METEOR) { // Lower Bound UID
                    DMX_Flags.RDMcheckUID_flag++;
                    if (*PD_Manu_ptr == METEOR && *PD_ID_ptr > DriverID) {
                        DMX_Flags.RDMcheckUID_flag = 0;
                    }
                }
                PD_Manu_ptr = PD_Read_u16ptr(); //PD    6~7
                PD_ID_ptr = PD_Read_u32ptr(); //PD    8~11
                if (*PD_Manu_ptr > METEOR) { // Upper Bound UID
                    DMX_Flags.RDMcheckUID_flag++;
                    if (*PD_Manu_ptr == METEOR && *PD_ID_ptr < DriverID) {
                        DMX_Flags.RDMcheckUID_flag = 0;
                    }
                }
                if (DMX_Flags.RDMcheckUID_flag == 3) { //if is in range...
//                    TX_PIN = 1; // Set pin high for MAB
                    RXTX_SWITCH_PIN = 1; //Set switch pin to TX mode
//                    TXREG = 0xFE; //disc_first char
                    BytePtr= &DISCOVERY_RDM_Data[DiscoveryLength];   // Point to Last byte  , use discovery packet
                    TxCount = 0; // Clear the address counter
                    TxState = TX_DISCOVERY; //TX send start discovery packet
                    RCIE = 0; //Disable RC interrupt
                    //abandon first 0xfe char
                    TXEN = 1; // Re-enable EUSART control of pin     
//                    TXIE = 1; // Re-Enable EUSART Interrupt     , it will send RDM_StartCode (0xCC)
                    TMR1 = TMR_LOAD_DISC_MAB; // Load the TMR_LOAD_MAB_DISCOVERY time 
                    TimerState = TIMER_DISC_MAB; // Next state is MAB_DISCOVERY end
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
//            TX_RDM_Data.PDL=2;//Control Field (2 slot)
//            PD.u16[TX_PD_u16-1] = 0;
            RDM_TXSTART();
            break;
        default:
            break;
    }
}

void RDM_TXSTART(void){
//    TXREG = E120_SC_RDM;
    TX_RDM_Response_Set();
    TX_RDM_Data.CS=RDM_get_checkSum(TX_RDM_Data,TX_PD_LEN);
    TxState = TX_START;
    RDM_tx_TimerBreak(); //100us Break... and Start to Response Discovery Signal
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

void PD_init(void){
    TX_RDM_Data.PDL=0x0;
    PackCount=0;
    PDCount=0;
}

void PD_write_u8(uint8_t u8){
    TX_RDM_Data.PDL++;
    PD.u8[TX_PD_LEN-TX_RDM_Data.PDL]=u8;
}

void PD_write_u16(uint16_t u16){
    TX_RDM_Data.PDL+=2;
    PD.u16[TX_PD_u16-(TX_RDM_Data.PDL>>1)]=u16;
}

void PD_write_u32(uint32_t u32){
    TX_RDM_Data.PDL+=4;
    PD.u32[TX_PD_u16-(TX_RDM_Data.PDL>>2)]=u32;
}

uint8_t* PD_Read_u8ptr(void){
    PackCount+=1;
    return &PD.u8[PD_LEN-PackCount];
}
uint16_t* PD_Read_u16ptr(void){
    PackCount+=2;
    return &PD.u16[PD_u16-(PackCount>>1)];
}
uint32_t* PD_Read_u32ptr(void){
    PackCount+=4;
    return &PD.u32[PD_u32-(PackCount>>2)];
}