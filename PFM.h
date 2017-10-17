
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded. 
#include <stdint.h>


extern uint16_t PFM_Read(uint16_t);

extern void PFM_Write(uint16_t ,uint16_t);


//High-Endurance Flash Memory Address Range 0x1F80~0x1FFF
//Erase per 0x20, please use belows:
//0x1F80~0x1F9F, 0x1FA0~0x1FBF, 0x1FC0~0x1FDF, 0x1FE0~0x1FFF


//enum{
//    Flash_DMXAddress=0x1F80,
//    Flash_UID1=0x1FA0,
//    Flash_UID2=0x1FA1,
//    Flash_UID3=0x1FA2,
//    Flash_UID4=0x1FA3,
//    Flash_Personality=0x1FC0
//};

enum{
    Flash_DMXAddress=0x0C80,
    Flash_UID1=0x0CA0,
    Flash_UID2=0x0CA1,
    Flash_UID3=0x0CA2,
    Flash_UID4=0x0CA3,
    Flash_Personality=0x0CC0
};

