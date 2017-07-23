/*
 * File:   covar014_lab4_v001.c
 * Author: daniellecovarrubias
 *
 * Created on February 26, 2017, 3:18 PM
 */

#include <p24Fxxxx.h>
#include <xc.h>
#include "stdint.h"
#include "covar014_lab2b_asmlib_v001.h"


#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSECME           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config SOSCSEL = SOSC           // Sec Oscillator Select (Default Secondary Oscillator (SOSC))
#pragma config WUTSEL = LEG             // Wake-up timer Select (Legacy Wake-up Timer)
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

#define FRAME_PER 10 //Period (1/FrameRate) in milliseconds

void initServo(void){
    
    CLKDIVbits.RCDIV = 0;
    AD1PCFG = 0x9fff; // all digital
    TRISB = 0b0000001000000000; //RP9 input
    LATB = 0;
    
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR3bits.RP6R = 18; // Use Pin RP6 for Output Compare 1 = "18" (Table 10-3)
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS
    
    //Timer3 setup
    T3CON = 0; //Stops any 16-bit Timer3 operation
    TMR3 = 0; //Clear contents of the timer3 register3
    PR3 = 40000;
    _T3IF = 0;
    T3CONbits.TCS = 0;
    T3CONbits.TGATE = 0;
    T3CONbits.TCKPS1=0;
    T3CONbits.TCKPS0=1; //prescalar of 8
    T3CONbits.TON = 1;  //Start Timer 3
    
    //OC1 registers setup
    OC1CONbits.OCM = 0b110; // Output compare PWM w/o faults
    OC1CONbits.OCTSEL = 1;  // Use Timer 3 for compare source
    OC1R=10000;
    OC1RS=10000;
};

void initRangeFinder(void){
    //peripherals
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPINR7bits.IC1R = 9; // Use Pin RP9 = "9", for Input Capture 1 (Table 10-2)
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock PPS

    _ICTMR=1; //timer 2
    
    
    //Timer2
    TMR2 = 0;
    T2CON = 0; 
    T2CONbits.TCKPS1=0;
    T2CONbits.TCKPS0=1; //prescalar of 8
    PR2 = 40000;  
    _T2IF = 0;
    T2CONbits.TON = 1;
};

void iLEDsetup(void){
    TRISA = 0b1111111111111110;
    LATA = 0x0000;
}
void writeColor(int r, int g, int b);

void drawFrame(unsigned int frame){
    int red;
    int green;
    red=frame;
    green= 255-frame;
    writeColor(red,green,0);
}
void writeColor(int r, int g, int b){
    //red
    int i; 
    for (i=0; i<=7; i++){
        if (0b10000000 & r){
            write_1();
        }
        else {
            write_0();
        }      
        r = r<<1;
    }
    
    //green
    int j=0;
    for (j=0; j<=7; j++){
        if (0b10000000 & g){
            write_1();
        }
        else {
            write_0();
        }    
        
        g = g<<1;
    }
    
    //blue
    int k=0;
    for (k=0; k<=7; k++){
        if (0b10000000 & b){
            write_1();
        }
        else {
            write_0();
        }    
        
        b = b<<1;
    }
    
    LATA = 0;
    delay50us();
}


void delay(int delay_in_ms){
    while (delay_in_ms>=0){
        delay1ms();
        delay_in_ms--;
    }
}


void setServo(int Val){
    
    OC1RS = Val;
    
}

unsigned int measRangeFinder(void){
    long unsigned int firstEdge;
    long unsigned int secondEdge;
    
    
    //clean out input capture register
    T2CONbits.TON = 0; //Stops 16-bit Timer2
    _ICM = 0x000; // Capture off
    
    int garbage; 
    while(_ICBNE == 1) {
        //Clear FIFO for IC1, max 4 reads
        garbage = IC1BUF;
    }
    TMR2 = 0;     // Initialize timer2 to zero
    T2CONbits.TON = 1; //Starts 16-bit Timer2
    _ICM = 0x001; // Capture rising and falling edges
    
    TRIGtoggle();
    
    delay(15);
    
    firstEdge = IC1BUF;
    secondEdge = IC1BUF;
    long unsigned int width = secondEdge - firstEdge;  // pulse width in TMR2 periods
    long unsigned int distance_in_cm= (width * 62.5) / 58;
    
    return distance_in_cm;
}


int main(void) {
    initServo();
    initRangeFinder();
    iLEDsetup();
    
        while(1){
            //setServo(2400);
            int frame;
            unsigned int distance = measRangeFinder();
            
            if (distance<15 && distance>2){
                frame=distance*20;
                drawFrame(frame);
            }
            else if (distance<2){
                frame=255;
                drawFrame(frame);  
            }
            else {
                frame=0;
                drawFrame(frame);   
            }
            
           
            
            
         
            
        }
    return 0;
    
    
    
}
