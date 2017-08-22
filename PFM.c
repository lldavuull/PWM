/*
 * File:   PMF.c
 * Author: METEOR
 *
 * Created on 2017?6?12?, ?? 4:46
 */


#include <xc.h>
#include "PFM.h"
#include "DMX.h"

uint16_t PFM_Read(uint16_t AddrPFM){
    
//    CFGS=0;         //Not configuration space
    PMADR=AddrPFM;
    RD=1;
    asm("nop");
    asm("nop");
    return  PMDAT;
}

void PFM_Write(uint16_t AddrPFM,uint16_t Data){
    do{
        GIE=0;          //Disable interrupts so required sequences will execute properly
        CFGS=0;         //Not configuration space
        PMADR=AddrPFM;
        
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

    }while(PFM_Read(AddrPFM)!=Data);       //Write Verify
    WREN=0;
    GIE=1;
}
