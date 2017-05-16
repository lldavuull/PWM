/*
 * File:   main.c
 * Author: METEOR
 *
 * Created on 2017?5?5?, ?? 10:37
 */


#include <xc.h>

/** Select the size of the receive buffer.
    This is the number of DMX512 Channels you will recieve
    For example a RGBW fixture may use 4 channels where a scanner may need 10 */
#define RX_BUFFER_SIZE 4

/** Selects the ms between breaks that sets the timeout flag.
    This is usually just over 1 second and is common for DMX Fixtures
    to go into a blackout or default mode if this occurs (i.e. signal lost) */
#define DMX_RX_TIMEOUT_MS 1200

char RGBW[4];
/** RxData Receive Data Buffer  */
volatile char RxData[RX_BUFFER_SIZE];
/** RxChannel Base Address / Channel to start reading from */
int         DMX_Address=1;
/** RxStart The Start Code for a valid data stream */
char          DMX_StartCode=0;
/** AddrCount Address counter used in the interrupt routine */
volatile int RxAddrCount=0;
/** *RxDataPtr Pointer to the receive data buffer. Used in interrupt */
volatile char *RxDataPtr;
/** RxState Current state in the receive state machine */
volatile char RxState=0;
/** RxTimer Counts ms since last overflow - used for 1 second timeout */
volatile int RxTimer=0;


 enum
 {
    RX_WAIT_FOR_BREAK,
    RX_WAIT_FOR_START,
    RX_READ_DATA
 };
 
 

/** DMX_FLAGS <BR> */
typedef struct
{
    // Recieve Flags
    /**  Indicates new data was recieved */
    unsigned int RxNew    :1;
    /**  Indicate valid break detected */
    unsigned int RxBreak  :1;
    /**  Indicates valid start is detected so store data */
    unsigned int RxStart  :1;
    /** Indicates 1 second timout occured */
    unsigned int RxTimeout :1;

    // Transmit Flags
    /** Indicated transmission is in progress */
    unsigned int TxRunning :1;
    /** Break active */
    unsigned int TxBREAK   :1;
    /**  Mark after Break Active */
    unsigned int TxMAB     :1; 

    /** Indicates a transmission is complete (Last Byte loaded into buffer) */
    unsigned int TxDone   :1;

    /** DMX_FLAGS Used to pass information from the ISR */
}DMX_FLAGS;

/** DMX_Flags Set in the ISR to indicate to helper functions etc */
volatile DMX_FLAGS Flags;




enum
{
    TIMER_1MS,
    TIMER_BREAK,
    TIMER_MAB,
    TIMER_FILL
};

volatile char TimerState=0;

typedef struct 
{
    union
    {
        struct
        {
            unsigned int MS    :1;
            unsigned int SEC   :1;
            unsigned int MIN   :1;
            unsigned int HR    :1;

            unsigned int BREAK :1;  // Start BREAK Timer
            unsigned int MAB   :1;  // Start MAB Timer
            unsigned int spare :2;
        };
        char flags;
    };

    int MS_Count;
    char  SEC_Count;
    char  MIN_Count;
    char  HR_Count;


}DMX_TIMER_DATA;

volatile DMX_TIMER_DATA Timer;


__CONFIG(FOSC_INTOSC & WDTE_OFF & PWRTE_OFF & MCLRE_ON & CP_OFF & BOREN_OFF & CLKOUTEN_OFF );
__CONFIG(WRT_OFF & PPS1WAY_OFF & PLLEN_ON & STVREN_ON & LPBOREN_OFF &  LVP_OFF );

#define PWMxCON_SET  0b10000000        // Enhanced features off
#define PRx_SET      0xFF              // Max resolution
#define TxCON_SET    0b00000101        // Post=1:1, ON, PRE=1:4
void ADC_interrupt(void);
void DMX_interrput(void);
void timer_interrupt(void);
void Sweep_PWM(void);

char led=253;//pwm number
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
volatile char RxAr[450];
volatile char *RxArPtr;
volatile int i=0;
void main(void) {


    PWM1CON=PWM2CON=PWM3CON=PWM4CON=PWMxCON_SET;
    PR2=PRx_SET;
    T2CON=TxCON_SET;
    
    OSCCON= 0b11111000;     // 4xPLL,16MHz, Config bits determine source  //   PLL=Phase-locked
    OSCTUNE=0b000000;
    
    TRISA2=TRISC0=TRISC1=TRISC2=0; //RA2, RC0, RC1, RC2 set to output
    ANSA2=ANSC0=ANSC1=ANSC2=0; //RA2, RC0, RC1, RC2 set to I/O
    TRISC3=0;ANSC3=0;   //RC3  is for interrupt test
    
    RA2PPS=0b0011; //PWM1_out
    RC0PPS=0b0100; //PWM2_out
    RC1PPS=0b0101; //PWM3_out
    RC2PPS=0b0110; //PWM4_out
    
    PWM1DCH=0x0F;
    PWM2DCH=0x0C;
    PWM3DCH=0x09;
    PWM4DCH=0x06;
    
    PWM1DCL=0xFF;
    PWM2DCL=0x7E;
    PWM3DCL=0x0D;
    PWM4DCL=0x9C;
    
    PWM1PHH=PWM2PHH=PWM3PHH=PWM4PHH=0x00;
    PWM1PHL=PWM2PHL=PWM3PHL=PWM4PHL=0x00;
    PWM1PRH=PWM2PRH=PWM3PRH=PWM4PRH=0x1F;
    PWM1PRL=PWM2PRL=PWM3PRL=PWM4PRL=0xFF;
    PWM1CLKCON=PWM2CLKCON=PWM3CLKCON=PWM4CLKCON=0b00000000;  //1:1
    
    

//    OPTION_REG=0b00000101;//Timer0 set Timer,  1:64  Period=1.024ms
    INTCON=0b11000000;//GIE=1; TMR0 interrupt=0; PEIE interrupt=1;
    
    
    // DMX UART START
    RXPPS=0b10101;  //RX=RC5
    TRISC5=1;       //set RC5 as input
    
    RCSTA=0b10010000; //enable RX  ; 8bit
    SYNC=0;// UART Enable
    BAUDCON=0b00000000; //BRG16 =0
    BRGH=1;             //High Buad Speed
    SPBRGH=0x00;
    SPBRGL=0x3;        //  16M/(16*(SPBRG+1)) = 250k
    RCIE=1;     //Enable RC interrupt
    RxArPtr = &RxAr[0];
    // DMX UART END

//    RC4PPS=0b1001; //RC4=TX
//    TXSTA=0b01000101;//disable TX,
    
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
    RC3=0;
     while(1)
     {

        if(Flags.RxNew==1)
        {
            Flags.RxNew=0;
//            dmx_read(0,RGBW,4); // Read the data
//            led_set_rgbw(RGBW[0],RGBW[1],RGBW[2],RGBW[3]);
            PWM1DCH=((char)(PWM[RxData[0]]>>8));
            PWM1DCL=((char)PWM[RxData[0]]);
            PWM2DCH=((char)(PWM[RxData[1]]>>8));
            PWM2DCL=((char)PWM[RxData[1]]);
            PWM3DCH=((char)(PWM[RxData[2]]>>8));
            PWM3DCL=((char)PWM[RxData[2]]);
            PWM4DCH=((char)(PWM[RxData[3]]>>8));
            PWM4DCL=((char)PWM[RxData[3]]);
            PWM1LDCON=PWM2LDCON=PWM3LDCON=PWM4LDCON=0b10000000;
        }
//        if(i>449){
//            GIE=0;
//        }
        
//        if(Timer.MS)
//        {
//            Timer.MS=0;
//            // If no data received for 1200ms turn the lights off
//            if(Flags.RxTimeout==1)
//            {
////                 led_set_rgbw(0,0,0,0);
//
//            }
//        }
     }
    
}

void interrupt isr(void)
{
//    ADC_interrupt();
//    Sweep_PWM();
    DMX_interrput();
//    timer_interrupt();

}

//volatile int j=0;


void DMX_interrput(void){
    if(RCIE & RCIF){
//        char d9;
//        d9=RX9D;
        volatile char RxDat;
        
        if(FERR) // if get error bit, clear the bit ;  occur at space for "break"
        {
           RxDat=RCREG;  // Clear the Framing Error - do not read before the bit test
           Flags.RxBreak=1;                 // Indicate a break
           RxState=RX_WAIT_FOR_START;
           RxTimer=0;
//           j++;
        }
        
        

        
        switch(RxState)
        {
            case RX_WAIT_FOR_BREAK:
                RxDat=RCREG;   // Just keep clearing the buffer until overflow.
                break;
                
            case RX_WAIT_FOR_START:
                
                 if(RCIF)   // make sure there is data avaliable (ie not a break)
                {
                    
                    RxDat=RCREG;
                    
                    if(RxDat==DMX_StartCode)
                    {
                        // Valid Start Received
                        RxState = RX_READ_DATA;
                        RxDataPtr = &RxData[0]; // Point to Buffer
                        RxAddrCount = 1;            // Reset current addr - Start at address 1! (zero is OFF)
                        Flags.RxStart = 1;          // Indicate a Start
                    }
                    else
                    {
                        RxState=RX_WAIT_FOR_BREAK;
                    }
                }
                break;

            case RX_READ_DATA:
                RxDat=RCREG;
//                RC3=~RC3;
//                *RxArPtr=RxAddrCount;
//                RxArPtr++;
//                *RxArPtr=DMX_Address;
//                RxArPtr++;
//                *RxArPtr=DMX_Address + RX_BUFFER_SIZE;
//                RxArPtr++;
//                i++;
                if(RxAddrCount >= DMX_Address && (DMX_Address !=0) )  // A selection of channel zero is "OFF"
                {
                    *RxDataPtr=RxDat;
                    RxDataPtr++;
                }
                
                RxAddrCount++;

                // Check for end of data packet we can read
                if(RxAddrCount >= (DMX_Address + RX_BUFFER_SIZE) && DMX_Address !=0 )
                {
                    Flags.RxNew=1;
                    RxState=RX_WAIT_FOR_BREAK;
                    RxTimer = 0;
                    Flags.RxTimeout=0;
                }
                break;
        }

        if(RxTimer > DMX_RX_TIMEOUT_MS)
        {
            Flags.RxTimeout=1;
            RxTimer=0;
        }
    }
}


void Sweep_PWM(void){
    if(TMR0IF){
        
        if((count<delay) && ((led>0) && (led<255)) ){
            count+=1;
        }else if((count<1000*delay )&& ((led== 0) || (led==255)) ){
            count+=1;
        }else{
            count=0;
            if(bright==1){
                led+=1;
            }
            else{
                led-=1;
            }
            if(led==255){
                bright=0;
            }else if(led==0){
                bright=1;
            }
            PWM1DCH=((char)(PWM[led]>>8));
            PWM1DCL=((char)PWM[led]);
            PWM1LDCON=0b10000000;
        }
        TMR0IF=0;
    }
}

//void timer_interrupt(void)
//{
//    if(TMRIE && TMRIF)
//    {
//        TMRIF=0;
//
//        switch(TimerState)
//        {
//            default:
//                TimerState=TIMER_1MS;
//
//            case TIMER_1MS:
//
//                TMR = TMR_LOAD_1MS;
//
//
//                RxTimer++;          // Inc timeout counter for receiver
//                if(RxTimer == DMX_RX_TIMEOUT_MS)
//                {
//                    RxTimer = DMX_RX_TIMEOUT_MS + 1;
//                    Flags.RxTimeout=1;
//                }
//
//
//                Timer.MS_Count++;            // Inc the MS Counter
//                Timer.MS=1;                  // Set the ms flag
//                if(Timer.MS_Count==1000)     // Check for 1 second
//                {
//                    Timer.MS_Count=0;
//                    Timer.SEC_Count++;
//                    Timer.SEC=1;
//                    if(Timer.SEC_Count==60)  // Check for Minute
//                    {
//                        Timer.SEC_Count=0;
//                        Timer.MIN_Count++;
//                        Timer.MIN=1;
//
//                        if(Timer.MIN_Count==60) // Check for Hour
//                        {
//                            Timer.MIN_Count=0;
//                            Timer.HR_Count++;
//                            Timer.HR=1;
//                        }
//                    }
//                }
//                break;
//        }
//    }
//}


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


