
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


//High-Endurance Flash Memory Address Range 0x0F80~0x0FFF
//0x0F80~0x0F9F, 0x0FA0~0x0FBF, 0x0FC0~0x0FDF, 0x0FE0~0x0FFF
enum{
    Flash_DMXAddress=0x0F80,
    Flash_UID1=0x0FA0,
    Flash_UID2=0x0FA1,
    Flash_UID3=0x0FA2,
    Flash_UID4=0x0FA3,
};