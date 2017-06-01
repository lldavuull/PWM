/*
 * File:   ADC.c
 * Author: METEOR
 *
 * Created on 2017?5?24?, ?? 3:58
 */


#include <xc.h>
#include "PWM.h"
#include "ADC.h"
#include "DMX.h"

void ADC_init(){
    
    //ADC Start//
//    ADIE = 1; //Enable Analog-to-Digital Converter (ADC) Interrupt 
    //    PWM1IE=1;
    TRISA4 = 1; //RA4 set to ADC (Analog to Digital Convert)
    ANSA4 = 1; //RA4 set to Analog input
    ADCON0 = 0b00001101; //SET RA4 (AN3) as ADC input and enable ADC
    ADCON1 = 0b01110000; // Left justified. FRC , VDD, VSS
    //    ADCON2=0b0001;// set PWM1_interrupt
//    GO_nDONE = 1; //Start to convert Analog to Digit
    ADIF = 0; // interrupt_flag clear

//    IOCIE =IOCAN5 = IOCAP5 = 1; //Enable the interrupt-on-change at RA5
//        RC3=0;
    //ADC End//
}

void ADC_loop(){
    if(ADC_Flags.New==1){
        RxTimer = 0;
        ADC_Flags.New = 0;
        PWM1DCH=PWM2DCH=PWM3DCH=PWM4DCH = PWM.PWM[ADC_Data].DCH;
        PWM1DCL=PWM2DCL=PWM3DCL=PWM4DCL = PWM.PWM[ADC_Data].DCL;
        PWM1LDCON=PWM2LDCON=PWM3LDCON=PWM4LDCON = 0b10000000;
        GO_nDONE = 1;   //Start to convert ADC
    }
}

void ADC_interrupt() {
    
    if (ADIF == 1) {
        
        ADC_Data=ADRESH;
        ADC_Flags.New=1;
        ADIF = 0;
    }
}