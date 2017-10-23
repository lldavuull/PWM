/*
 * File:   PMF.c
 * Author: METEOR
 *
 * Created on 2017?6?12?, ?? 4:46
 */


#include <xc.h>
#include "PFM.h"
#include "DMX.h"
#include "RDM.h"

uint16_t PFM_Read(uint16_t AddrRd){
//    CFGS=0;         //Not configuration space
    PMADR=AddrRd;
    RD=1;
    asm("nop");
    asm("nop");
    return  PMDAT;
}

void PFM_Erase(uint16_t AddrErs){
    GIE=0;          //Disable interrupts so required sequences will execute properly
    CFGS=0;         //Not configuration space
    PMADR=AddrErs;
    //
    FREE = 1;       //Program Flash Erase Operation
    WREN = 1;
    PFM_Unlock();
    //
    WREN=0;
    GIE=1;
}

void PFM_Write(uint16_t AddrWrt, uint16_t Data){
    do{
        GIE=0;          //Disable interrupts so required sequences will execute properly
        CFGS=0;         //Not configuration space
        PMADR=AddrWrt;
        
        //
        FREE = 1;       //Program Flash Erase Operation
        WREN = 1;
        PMCON2=0x55;
        PMCON2=0xAA;
        WR=1;
        asm("nop");//NOP instructions are forced as processor
        asm("nop");
        
        //
        FREE = 0;       //Program Flash Write Operation
        LWLO = 1;    //LWLO= load address and write on the next WR command  ;  WREN=Enable writes;
        PMDAT=Data;     //Load data byte
        LWLO=0;
        PMCON2=0x55;
        PMCON2=0xAA;
        WR=1;
        asm("nop");//NOP instructions are forced as processor
        asm("nop");
        
    }while(PFM_Read(AddrWrt)!=Data);       //Write Verify
    
    WREN=0;
    GIE=1;
//    
//    do{
//        GIE=0;          //Disable interrupts so required sequences will execute properly
//        CFGS=0;         //Not configuration space
//        PMADR=AddrPFM;
//        //
//        FREE = 1;       //Program Flash Erase Operation
//        WREN = 1;
//        PFM_Unlock();
//        //
//        FREE = 0;       //Program Flash Write Operation
//        LWLO = 1;       //LWLO= load address and write on the next WR command  ;  WREN=Enable writes;
//        PMDAT=Data;     //Load data byte
//        LWLO=0;
//        //
//        PFM_Unlock();
//        //
//    }while(PFM_Read(Addr)!=Data);       //Write Verify
//    WREN=0;
//    GIE=1;
}


void PFM_write_String(uint16_t AddrWrtStr, uint8_t* u8ptr, uint8_t u8len){
    
    GIE=0;          //Disable interrupts so required sequences will execute properly
    CFGS=0;         //Not configuration space
    PMADR=AddrWrtStr;
    //
    FREE = 1;       //Program Flash Erase Operation
    WREN = 1;
    PMCON2=0x55;
    PMCON2=0xAA;
    WR=1;
    asm("nop");//NOP instructions are forced as processor
    asm("nop");
    //
    FREE = 0;       //Program Flash Write Operation
    LWLO = 1;       //LWLO= load address and write on the next WR command  ;  WREN=Enable writes;
    //
    ReadCount=0;
    while(1){
        PMDAT=*u8ptr;     //Load data byte
        ReadCount++;
        if(ReadCount<u8len){
            PFM_Unlock();
            u8ptr++;
            PMADR++;
        }else{
            break;
        }
        
    }
    //
    LWLO=0;
    PFM_Unlock();
    WREN=0;
    GIE=1;
}

uint8_t PFM_Read_String(uint16_t AddrRdStr, uint8_t* dataptr){
    ReadCount=0;
    while(ReadCount<32){
        tmp16=PFM_Read(AddrRdStr);
        if(tmp16==0x3fff){
            break;
        }else{
            *dataptr=tmp16;
            ReadCount++;
            dataptr++;
            AddrRdStr++;
        }
        
    }
    return ReadCount;
}

void PFM_Unlock(){
    PMCON2=0x55;
    PMCON2=0xAA;
    WR=1;
    asm("nop");//NOP instructions are forced as processor
    asm("nop");
}