
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include <xc.h> // include processor files - each processor file is guarded. 
#include <stdint.h>

extern void PFM_Unlock();
extern uint16_t PFM_Read(uint16_t);
extern void PFM_Erase(uint16_t);
extern void PFM_Write(uint16_t ,uint16_t);
extern void PFM_write_String(uint16_t,uint8_t*,uint8_t);
extern uint8_t PFM_Read_String(uint16_t,uint8_t*);

//High-Endurance Flash Memory Address Range 0x1F80~0x1FFF
//Erase per 0x20, please use belows:
//0x1F80~0x1F9F, 0x1FA0~0x1FBF, 0x1FC0~0x1FDF, 0x1FE0~0x1FFF


enum{
    Flash_DMX_ADDRESS=0x1F80,
    Flash_PERSONALITY=0x1FA0,
    Flash_DEVICE_LABEL=0x1FC0,
};

//enum{
//    Flash_DMX_ADDRESS=0x07E0,
////    Flash_UID=0x0CA0,
//    Flash_PERSONALITY=0x06C0,
//    Flash_DEVICE_LABEL=0x06E0,
//};

