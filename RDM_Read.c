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
            PD_write_u8(DMX_Flags.RDM_Identify_Device);// 1= True, 0=False
            break;
        case E120_DEVICE_INFO:
            PD_write_u16(E120_PROTOCOL_VERSION);//RDM Protocol Version  //uint16_t
            PD_write_u16(E120_DEVICE_MODEL_DESCRIPTION);//Device Model ID
            PD_write_u16(E120_PRODUCT_DETAIL_LED);//Product Category
            PD_write_u32(BOOT_SOFTWARE_VERSION_ID);//SoftWareVersion (uint32)
            PD_write_u16(FOOTPRINT);//FootPrint
            PD_write_u8(PERSONALITY);//Personality
            PD_write_u8(PERSONALITY_SIZE);//Personality Size
            PD_write_u16(DMX_Address);//StartAddress
            PD_write_u16(0);//Sub-Device Count
            PD_write_u8(0);//Sensor Count
            break;
        case E120_DMX_START_ADDRESS:
            PD_write_u16(DMX_Address);
            break;
        case E120_SOFTWARE_VERSION_LABEL:
            PD_write_String(&SOFTWARE_VERSION_LABEL[0],SOFTWARE_VERSION_LABEL_SIZE);
            break;
        case E120_BOOT_SOFTWARE_VERSION_ID:
            PD_write_u32(BOOT_SOFTWARE_VERSION_ID);
            break;
        case E120_BOOT_SOFTWARE_VERSION_LABEL:
            PD_write_String(&BOOT_SOFTWARE_VERSION_LABEL[0],BOOT_SOFTWARE_VERSION_LABEL_SIZE);
            break;
        case E120_SUPPORTED_PARAMETERS:
            while(ReadCount<SUPPORTED_PARAMETERS_SIZE){
                PD_write_u16(SUPPORTED_PARAMETERS[ReadCount]);
                ReadCount++;
            }
            break;
        case E120_PARAMETER_DESCRIPTION:
            tmp16=*PD_Read_u8ptr();//Read PID # Requested  ; The manufacturer specific PID requested by the controller. Range 0x8000 to 0xFFDF
            PD_write_u16(tmp16);//PID # Requested
            PD_write_u8(0x0001);//PDL Size
            PD_write_u8(E120_DS_ASCII);//Data Type
            PD_write_u8(E120_CC_GET);//Command Class
            PD_write_u8(0x00);//Type  ; This field no longer has any meaning and should be filled with 0x00 in the response. Controllers should ignore the contents of this field.
            PD_write_u8(E120_UNITS_NONE);//Unit ; Unit is an unsigned 8-bit value enumerated by Table A-13. It defines the SI (International System of units) unit of the specified PID data.
            PD_write_u8(E120_PREFIX_NONE);//Prefix ; Prefix is an unsigned 8-bit value enumerated by Table A-14. It defines the SI Prefix and multiplication factor of the units
            PD_write_u32(0x00000000);//Min Valid Value
            PD_write_u32(0xffffffff);//Max Valid Value
            PD_write_u32(0x00000000);//Default Value (32-bit)
            break;
        case E137_1_IDENTIFY_MODE:
            PD_write_u8(IDENTIFY_MODE); //Intensity of IDENTIFY
            break;
        case E120_DMX_PERSONALITY:
            PD_write_u8(PERSONALITY);//Current Personality
            PD_write_u8(PERSONALITY_SIZE);// Total of Personalities
            break;
        case E120_DMX_PERSONALITY_DESCRIPTION:
            Addr=*PD_Read_u8ptr();//Read Personality Requested
            PD_write_u8(Addr);//Write Personality Requested
            PD_write_u16(PERSONALITY_DEFINITIONS[Addr-1].footprint);//Write Personality footprint
            PD_write_String( PERSONALITY_DEFINITIONS[Addr-1].description , PERSONALITY_DEFINITIONS[Addr-1].descriptionsize );
            break;
        case E120_PRODUCT_DETAIL_ID_LIST:
            PD_write_u16(E120_PRODUCT_DETAIL_LED);
            break;
        case E120_DEVICE_MODEL_DESCRIPTION:
            PD_write_String(&DEVICE_MODEL_DESCRIPTION[0] , DEVICE_MODEL_DESCRIPTION_SIZE);
            break;
        case E120_MANUFACTURER_LABEL:
            PD_write_String(&MANUFACTURER_LABEL[0] , MANUFACTURER_LABEL_SIZE);
            break;
        case E120_DEVICE_LABEL:
            PD_write_String(&DEVICE_LABEL[0] , DEVICE_LABEL_SIZE);
            break;
        case E120_SLOT_INFO:
            if(PERSONALITY<5){
                while(ReadCount<PERSONALITY){
                    PD_write_u16(ReadCount);
                    PD_write_u8(SLOT_TYPE[ReadCount]);
                    PD_write_u16(SLOT_LABEL_ID[ReadCount]);
                    ReadCount++;
                }
            }else{
                ReadCount=4;
                while(ReadCount<PERSONALITY){
                    PD_write_u16(ReadCount);
                    PD_write_u8(SLOT_TYPE[ReadCount]);
                    PD_write_u16(SLOT_LABEL_ID[ReadCount]);
                    tmp8=ReadCount+1;
                    PD_write_u16(tmp8);
                    PD_write_u8(SLOT_TYPE[tmp8]);
                    PD_write_u16(SLOT_LABEL_ID[tmp8]);
                    ReadCount++;
                }
            }
            break;
        case E120_SLOT_DESCRIPTION:
            tmp16=*PD_Read_u16ptr();//Read Slot Requested
            PD_write_u16(tmp16);
            if(PERSONALITY<5){
                PD_write_String(SLOT_DEFINITIONS[0].description , SLOT_DEFINITIONS[0].descriptionsize);
                PD_write_u8(SLOT[tmp16]);
                PD_write_String(SLOT_DEFINITIONS[1].description , SLOT_DEFINITIONS[1].descriptionsize);
            }else{
                PD_write_String(SLOT_DEFINITIONS[0].description , SLOT_DEFINITIONS[0].descriptionsize);
                PD_write_u8(SLOT[tmp16]);
                if(tmp16%2==0){
                    PD_write_String(SLOT_DEFINITIONS[1].description , SLOT_DEFINITIONS[1].descriptionsize);
                }else{
                    PD_write_String(SLOT_DEFINITIONS[2].description , SLOT_DEFINITIONS[2].descriptionsize);
                }
            }
            break;
        case E120_DEFAULT_SLOT_VALUE:
            if(PERSONALITY<5){
                while(ReadCount<PERSONALITY){
                    PD_write_u16(ReadCount);
                    PD_write_u8(0);
                    ReadCount++;
                }
            }else{
                ReadCount=4;
                while(ReadCount<PERSONALITY){
                    PD_write_u16(ReadCount);
                    PD_write_u8(0);
                    tmp8=ReadCount+1;
                    PD_write_u16(tmp8);
                    PD_write_u8(0);
                    ReadCount++;
                }
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
            PFM_Erase(Flash_DMX_ADDRESS);
            PFM_Write(Flash_DMX_ADDRESS,DMX_Address);
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
            PFM_Erase(Flash_PERSONALITY);
            PFM_Write(Flash_PERSONALITY,PERSONALITY);
            FOOTPRINT=Get_FootPrint(PERSONALITY);
            break;
        case E120_DEVICE_LABEL:
            DEVICE_LABEL_SIZE=RX_RDM_Data.PDL;
            PD_Read_String(&DEVICE_LABEL[0] , DEVICE_LABEL_SIZE);
            PFM_write_String(Flash_DEVICE_LABEL,&DEVICE_LABEL[0],DEVICE_LABEL_SIZE);
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
                PD_Manu_ptr =PD_Read_u16ptr();
                PD_ID_ptr = PD_Read_u32ptr();
                
                if (*PD_Manu_ptr <= UID.MANC) { // Lower Bound UID
                    DMX_Flags.RDMcheckUID_flag++;
                    if (*PD_Manu_ptr == UID.MANC && *PD_ID_ptr > UID.ID) {
                        DMX_Flags.RDMcheckUID_flag = 0;
                    }
                }
                PD_Manu_ptr = PD_Read_u16ptr(); 
                PD_ID_ptr = PD_Read_u32ptr();
                if (*PD_Manu_ptr >= UID.MANC) { // Upper Bound UID
                    DMX_Flags.RDMcheckUID_flag++;
                    if (*PD_Manu_ptr == UID.MANC && *PD_ID_ptr < UID.ID) {
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
    RDM_tx_TimerBreak(); // Load value for MBB      ( 4us)
}

void TX_RDM_Response_Set(){
    TX_RDM_Data.DUID.ID=RX_RDM_Data.SUID.ID;
    TX_RDM_Data.DUID.M=RX_RDM_Data.SUID.M;
    TX_RDM_Data.TN=RX_RDM_Data.TN;
    TX_RDM_Data.CC=RX_RDM_Data.CC+1;
    TX_RDM_Data.PID=RX_RDM_Data.PID;
    TX_RDM_Data.ML=0x18+TX_RDM_Data.PDL;  
    TX_RDM_Data.PORT=E120_RESPONSE_TYPE_ACK; //0x00
}


uint16_t RDM_get_checkSum(RDM_Data Data, char len) {
    checkSum = 0x00CD; // 0xCC+ 0x01
    ReadCount = 23;
    while (ReadCount > 1) {
        checkSum += Data.value[ReadCount];
        ReadCount--;
    }
    ReadCount = len-1;
    tmp8 = len - Data.PDL;
    while (ReadCount >= tmp8) {
        checkSum += PD.u8[tmp8];
        tmp8++;
    }
    return checkSum;
}

void PD_init(void){
    TX_RDM_Data.PDL=0x0;
    ReadCount=0;
    tmp8=0;
}

void PD_write_u8(uint8_t u8){
    TX_RDM_Data.PDL++;
    PD.u8[TX_PD_LEN-TX_RDM_Data.PDL]=u8;
}

void PD_write_u16(uint16_t u16){
    TX_RDM_Data.PDL+=2;
    if(TX_RDM_Data.PDL%2==0){
        PD.u16[TX_PD_u16-(TX_RDM_Data.PDL>>1)]=u16;
    }else{
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL]=u16;
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL+1]=(u16>>8);
    }
}

void PD_write_u32(uint32_t u32){
    TX_RDM_Data.PDL+=4;
    if(TX_RDM_Data.PDL%4==0){
        PD.u32[TX_PD_u32-(TX_RDM_Data.PDL>>2)]=u32;
    }else{
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL]=u32;
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL+1]=(u32>>8);
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL+2]=(u32>>16);
        PD.u8[TX_PD_LEN-TX_RDM_Data.PDL+3]=(u32>>24);
    }
}

void PD_write_String(uint8_t* u8ptr, uint8_t u8len){
    tmp8=0;
    while(tmp8<u8len){
        PD_write_u8(*u8ptr);
        tmp8++;
        u8ptr++;
    }
}

uint8_t* PD_Read_u8ptr(void){
    ReadCount+=1;
    return &PD.u8[PD_LEN-ReadCount];
}
uint16_t* PD_Read_u16ptr(void){
    ReadCount+=2;
    return &PD.u16[PD_u16-(ReadCount>>1)];
}
uint32_t* PD_Read_u32ptr(void){
    ReadCount+=4;
    return &PD.u32[PD_u32-(ReadCount>>2)];
}

void PD_Read_String(uint8_t* u8ptr, uint8_t u8len){
    while(ReadCount<u8len){
        *u8ptr=*PD_Read_u8ptr();
        u8ptr++;
    }
}