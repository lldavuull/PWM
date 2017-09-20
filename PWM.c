/*
 * File:   PWM.c
 * Author: METEOR
 *
 * Created on 2017?5?24?, ?? 10:35
 */


#include <xc.h>
#include "PWM.h"


char led = 253; //pwm number
char bright = 1; //1=brighter  0=darker
const int delay = 2; // delay*1.024ms
int count = 0;

void Sweep_PWM_init(void){
    OPTION_REG=0b00000101;//Timer0 set Timer,  1:64  Period=1.024ms
    TMR0IE=1;// TMR0 interrupt=1; 
    PEIE=0;
}

void Sweep_PWM(void){
    if(TMR0IF){
        
        if((count<delay) && ((led>0) && (led<255)) ){
            count+=1;
        }else if((count<100*delay )&& ((led== 0) || (led==255)) ){
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
            PWM1DCH=PWM.PWM[led].DCH;
            PWM1DCL=PWM.PWM[led].DCL;
            PWM1LDCON=0b10000000;
        }
        TMR0IF=0;
    }
}