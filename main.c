


/*
 * File:   main.c
 * Author: METEOR
 *
 * Created on 2017?5?5?, ?? 10:37
 */


#include <xc.h>

__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & BOREN_OFF & CLKOUTEN_OFF );
__CONFIG(WRT_OFF & PPS1WAY_OFF & PLLEN_ON & STVREN_ON & LPBOREN_OFF &  LVP_OFF );

#define PWMxCON_SET  0b10000000        // Enhanced features off
#define PRx_SET      0xFF              // Max resolution
#define TxCON_SET    0b00000101        // Post=1:1, ON, PRE=1:4

void ADC_interrupt(void);
void DMX_interrput(void);
void Sweep_PWM(void);

char led=255;//pwm number
char bright=1; //1=brighter  0=darker
const int delay=1;  // delay*1.024ms
int count=0;
const int PWM[256]={
    
0x0000, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0046, 0x0048, 0x0048, 0x0048, 							
0x0048, 0x0049, 0x0049, 0x0049, 0x004A, 0x004A, 0x004C, 0x004D, 0x004D, 0x004E, 							
0x0050, 0x0052, 0x0053, 0x0054, 0x0056, 0x0058, 0x005A, 0x005C, 0x005E, 0x0060, 							
0x0062, 0x0064, 0x0067, 0x006A, 0x006C, 0x006F, 0x0072, 0x0076, 0x007A, 0x007C, 							
0x0080, 0x0083, 0x0088, 0x008A, 0x008E, 0x0092, 0x0098, 0x009E, 0x00A4, 0x00AC, 							
0x00B4, 0x00BC, 0x00C0, 0x00C8, 0x00D0, 0x00DE, 0x00E4, 0x00EC, 0x00F6, 0x0106, 							
0x010E, 0x0116, 0x011E, 0x012C, 0x0134, 0x013C, 0x0146, 0x0156, 0x0160, 0x0168, 							
0x0176, 0x0182, 0x018A, 0x0196, 0x01A6, 0x01B2, 0x01BE, 0x01CC, 0x01D8, 0x01E4, 							
0x01F6, 0x0202, 0x020E, 0x0220, 0x0230, 0x0240, 0x024C, 0x0262, 0x0270, 0x0280, 							
0x0292, 0x02A2, 0x02B6, 0x02C6, 0x02D6, 0x02EA, 0x0300, 0x031C, 0x0326, 0x0336, 							
0x0348, 0x0360, 0x0376, 0x0386, 0x039E, 0x03B0, 0x03C8, 0x03DC, 0x03F4, 0x0406, 							
0x041C, 0x043A, 0x044C, 0x0468, 0x047E, 0x0496, 0x04B2, 0x04C8, 0x04E0, 0x04F8, 							
0x0514, 0x0530, 0x054C, 0x0566, 0x0580, 0x059C, 0x05B8, 0x05D8, 0x05F4, 0x060E, 							
0x0628, 0x0648, 0x0668, 0x0688, 0x06A4, 0x06C4, 0x06DC, 0x0704, 0x0724, 0x0740, 							
0x0764, 0x0784, 0x07A4, 0x07C8, 0x07EC, 0x0810, 0x0830, 0x0850, 0x0870, 0x0898, 							
0x08C0, 0x08E8, 0x0908, 0x0930, 0x0958, 0x0980, 0x09A0, 0x09C8, 0x09F0, 0x0A18, 							
0x0A40, 0x0A68, 0x0A94, 0x0ABC, 0x0AE4, 0x0B10, 0x0B38, 0x0B60, 0x0B8C, 0x0BB4, 							
0x0BE8, 0x0C16, 0x0C40, 0x0C6C, 0x0C98, 0x0CCC, 0x0CFC, 0x0D28, 0x0D58, 0x0D88, 							
0x0DB8, 0x0DE8, 0x0E18, 0x0E48, 0x0E78, 0x0EA8, 0x0EE0, 0x0F10, 0x0F48, 0x0F7C, 							
0x0FB0, 0x0FE4, 0x1018, 0x1050, 0x1088, 0x10BC, 0x10F4, 0x112C, 0x1164, 0x1198, 							
0x11D0, 0x120C, 0x1244, 0x127C, 0x12B4, 0x12F0, 0x1328, 0x136C, 0x13A8, 0x13E4, 							
0x1420, 0x145C, 0x149C, 0x14DC, 0x1518, 0x1558, 0x159C, 0x15DC, 0x161C, 0x165C, 							
0x169C, 0x16E0, 0x1728, 0x1768, 0x17A8, 0x17F0, 0x1838, 0x1878, 0x18C0, 0x1904, 							
0x1948, 0x1998, 0x19E0, 0x1A28, 0x1A70, 0x1AB8, 0x1B00, 0x1B48, 0x1B90, 0x1BE0, 							
0x1C30, 0x1C80, 0x1CC8, 0x1D18, 0x1D68, 0x1D80, 0x1DB8, 0x1E08, 0x1E50, 0x1EA0, 							
0x1EE8, 0x1F30, 0x1F78, 0x1FC0, 0x1FE8, 0x1FFF							
        
};

void main(void) {


    PWM1CON=PWM2CON=PWM3CON=PWM4CON=PWMxCON_SET;
    PR2=PRx_SET;
    T2CON=TxCON_SET;
    
    OSCCON= 0b11111000;     // 4xPLL,8MHz(32MHz), Config bits determine source  //   PLL=Phase-locked
    OSCTUNE=0b000000;
    
    TRISA2=TRISC0=TRISC1=TRISC2=0; //RA2, RC0, RC1, RC2 set to output
    ANSA2=ANSC0=ANSC1=ANSC2=0; //RA2, RC0, RC1, RC2 set to I/O
    RA2PPS=0b0011; //PWM1_out
    RC0PPS=0b0100; //PWM2_out
    RC1PPS=0b0101; //PWM3_out
    RC2PPS=0b0110; //PWM4_out
    
    PWM1DCH=0x2F;
    PWM2DCH=0x07;
    PWM3DCH=0x09;
    PWM4DCH=0x06;
    
    PWM1DCL=0xFF;
    PWM2DCL=0xC8;
    PWM3DCL=0x0D;
    PWM4DCL=0x9C;
    
    PWM1PHH=PWM2PHH=PWM3PHH=PWM4PHH=0x00;
    PWM1PHL=PWM2PHL=PWM3PHL=PWM4PHL=0x00;
    PWM1PRH=PWM2PRH=PWM3PRH=PWM4PRH=0x1F;
    PWM1PRL=PWM2PRL=PWM3PRL=PWM4PRL=0xFF;
    PWM1CLKCON=PWM2CLKCON=PWM3CLKCON=PWM4CLKCON=0b00000000;  //1:1
    
    

    OPTION_REG=0b00000101;//Timer0 set Timer,  1:64  Period=1.024ms
    INTCON=0b10100000;//GIE=1; TMR0 interrupt=1
    
    
//    RXPPS=0b10101;  //RX=RC5
//    TRISC5=1;       //set RC5 as input
//    RCSTA=0b10010000; //enable RX
//    BAUDCON=0b00000000;
////    RC4PPS=0b1001; //RC4=TX
////    TXSTA=0b01000101;//disable TX, 
//    SPBRGH=0x00;
//    SPBRGL=0x01; //  32M/(64*(SPBRG+1)) = 250k
////    RCIE=1;     //Enable RC interrupt
//    
//    
//    ADIE=1;
////    PWM1IE=1;
//    
//    TRISC3=1;//RC3 set to ADC (Analog to Digital Convert)
//    ANSC3=1;//RC3 set to Analog input
//    ADCON0=0b00011101;//SET RC3 (AN7) as ADC input and enable ADC
//    ADCON1=0b11110000;// Right justify, FRC , VDD, VSS
////    ADCON2=0b0001;// set PWM1_interrupt
//    ADGO=1; //Start to convert Analog to Digit
//    ADIF=0; // interrupt_flag clear
    


    
     while(1)
     {
     }
    
}

void interrupt isr(void)
{
//    ADC_interrupt();
    Sweep_PWM();
//    DMX_interrput();
}

void Sweep_PWM(void){
    if(TMR0IF==1){
        if((count<delay) && ((led>0) && (led<255)) ){
            count+=1;
        }else if((count<200*delay ) && ((led== 0) || (led==255)) ){
            count+=1;
        }else{
            count=0;
            if(bright==1){
                led+=1;
            }
            else{
                led-=1;
            }
            PWM1DCH=((char)(PWM[led]>>8));
            PWM1DCL=((char)PWM[led]);
            PWM1LDCON=0b10000000;
            
            if(led==255){
                bright=0;
            }else if(led==0){
                bright=1;
            }


            
        }
                    TMR0IF=0;
    }
}

void DMX_interrput(void){
//    RCREG=;
}

void ADC_interrupt(void){
    if(ADIF==1){
        PWM1DCH=(ADRESH<<4);
        PWM1DCH|=(ADRESL>>4);
        PWM1DCL=(ADRESL<<4);
        ADIF=0;
        ADGO=1;
//        PWM1IF=0;
        IOCCF3=0;
        GIE=1;
    }
    
    
}



